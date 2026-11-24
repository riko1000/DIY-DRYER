const crypto = require("node:crypto");

const SESSION_COOKIE = "dryer_session";
const SESSION_LIFETIME_SECONDS = 12 * 60 * 60;
const DEVICE_ONLINE_WINDOW_MS = 25000;
const COMMAND_LIFETIME_MS = 15000;
const DEVICE_STATES = new Set(["Booting", "Idle", "Drying", "Finished", "Error"]);

function sendJson(res, status, value) {
    res.statusCode = status;
    res.setHeader("Content-Type", "application/json; charset=utf-8");
    res.setHeader("Cache-Control", "no-store");
    res.end(JSON.stringify(value));
}

function getSessionSecret() {
    const secret = process.env.SESSION_SECRET;
    if (!secret || Buffer.byteLength(secret) < 32) {
        throw new Error("SESSION_SECRET must contain at least 32 bytes");
    }
    return secret;
}

function createSessionToken() {
    const issuedAt = Math.floor(Date.now() / 1000).toString();
    const signature = crypto
        .createHmac("sha256", getSessionSecret())
        .update(issuedAt)
        .digest("hex");
    return `${issuedAt}.${signature}`;
}

function getCookie(req, name) {
    const cookies = (req.headers.cookie || "").split(";");
    for (const cookie of cookies) {
        const separator = cookie.indexOf("=");
        if (separator < 0) continue;
        if (cookie.slice(0, separator).trim() === name) {
            return cookie.slice(separator + 1).trim();
        }
    }
    return "";
}

function hasValidSession(req) {
    const token = getCookie(req, SESSION_COOKIE);
    const separator = token.indexOf(".");
    if (separator < 1) return false;

    const issuedAt = token.slice(0, separator);
    const signature = token.slice(separator + 1);
    const timestamp = Number(issuedAt);
    const now = Math.floor(Date.now() / 1000);

    if (!Number.isSafeInteger(timestamp) ||
        timestamp > now ||
        now - timestamp > SESSION_LIFETIME_SECONDS) {
        return false;
    }

    const expected = crypto
        .createHmac("sha256", getSessionSecret())
        .update(issuedAt)
        .digest();

    const supplied = Buffer.from(signature, "hex");
    return supplied.length === expected.length &&
        crypto.timingSafeEqual(supplied, expected);
}

function passwordMatches(provided, expected) {
    if (typeof provided !== "string" || !expected) return false;

    const providedBuffer = Buffer.from(provided);
    const expectedBuffer = Buffer.from(expected);
    return providedBuffer.length === expectedBuffer.length &&
        crypto.timingSafeEqual(providedBuffer, expectedBuffer);
}

function readBody(req) {
    if (req.body && typeof req.body === "object") return req.body;
    if (typeof req.body === "string") return JSON.parse(req.body);
    return {};
}

function getDeviceId() {
    const deviceId = process.env.DRYER_DEVICE_ID || "dryer-4338";
    if (!/^[a-zA-Z0-9_-]{1,64}$/.test(deviceId)) {
        throw new Error("DRYER_DEVICE_ID is invalid");
    }
    return deviceId;
}

function getSupabaseConfig() {
    const projectUrl = process.env.SUPABASE_URL;
    const secretKey = process.env.SUPABASE_SECRET_KEY;

    if (!projectUrl || !secretKey) {
        throw new Error("Supabase environment variables are missing");
    }

    let url;
    try {
        url = new URL(projectUrl);
    } catch {
        throw new Error("SUPABASE_URL is invalid");
    }

    if (url.protocol !== "https:") {
        throw new Error("SUPABASE_URL must use HTTPS");
    }

    return {
        baseUrl: url.origin,
        secretKey,
        deviceId: getDeviceId()
    };
}

async function supabaseRequest(path, method = "GET", body) {
    const config = getSupabaseConfig();
    const response = await fetch(`${config.baseUrl}/rest/v1/${path}`, {
        method,
        headers: {
            apikey: config.secretKey,
            "Content-Type": "application/json"
        },
        body: body === undefined ? undefined : JSON.stringify(body),
        signal: AbortSignal.timeout(5000)
    });

    const responseText = await response.text();
    let responseBody = null;
    if (responseText) {
        try {
            responseBody = JSON.parse(responseText);
        } catch {
            throw new Error("Supabase returned invalid JSON");
        }
    }

    if (!response.ok) {
        const error = new Error(`Supabase request failed (${response.status})`);
        error.statusCode = 502;
        throw error;
    }

    return responseBody;
}

function getDeviceToken() {
    const token = process.env.DEVICE_TOKEN;
    if (!token || Buffer.byteLength(token) < 32) {
        throw new Error("DEVICE_TOKEN must contain at least 32 bytes");
    }
    return token;
}

function hasValidDeviceToken(req) {
    const expected = Buffer.from(getDeviceToken());
    const authorization = req.headers.authorization || "";
    if (!authorization.startsWith("Bearer ")) return false;

    const supplied = Buffer.from(authorization.slice(7));
    return supplied.length === expected.length &&
        crypto.timingSafeEqual(supplied, expected);
}

function isFiniteNumber(value) {
    return typeof value === "number" && Number.isFinite(value);
}

function isValidTelemetry(status) {
    return status &&
        typeof status === "object" &&
        DEVICE_STATES.has(status.state) &&
        typeof status.heater === "boolean" &&
        typeof status.motor === "boolean" &&
        typeof status.running === "boolean" &&
        isFiniteNumber(status.target) &&
        status.target >= 0 && status.target <= 100 &&
        (status.heatbed === null || isFiniteNumber(status.heatbed)) &&
        (status.chamber === null || isFiniteNumber(status.chamber)) &&
        (status.humidity === null || isFiniteNumber(status.humidity)) &&
        typeof status.dhtConnected === "boolean" &&
        Number.isInteger(status.progress) &&
        status.progress >= 0 && status.progress <= 100 &&
        Number.isInteger(status.remainingSeconds) &&
        status.remainingSeconds >= 0 &&
        Number.isInteger(status.uptimeSeconds) &&
        status.uptimeSeconds >= 0;
}

function isValidAcknowledgement(ack) {
    return ack === null ||
        (ack &&
            typeof ack === "object" &&
            typeof ack.id === "string" &&
            ack.id.length > 0 &&
            ack.id.length <= 64 &&
            typeof ack.ok === "boolean");
}

function hasJsonContentType(req) {
    return (req.headers["content-type"] || "")
        .toLowerCase()
        .includes("application/json");
}

function isDeviceOnline(record) {
    if (!record || !record.last_seen) return false;
    const lastSeen = Date.parse(record.last_seen);
    const age = Date.now() - lastSeen;
    return Number.isFinite(lastSeen) &&
        age >= 0 &&
        age <= DEVICE_ONLINE_WINDOW_MS;
}

async function handleLogin(req, res) {
    if (req.method !== "POST") {
        res.setHeader("Allow", "POST");
        return sendJson(res, 405, { error: "Method not allowed" });
    }

    if (!process.env.DASHBOARD_PASSWORD ||
        Buffer.byteLength(process.env.DASHBOARD_PASSWORD) < 16) {
        return sendJson(res, 503, {
            error: "Dashboard password must be configured with at least 16 characters"
        });
    }

    let body;
    try {
        body = readBody(req);
    } catch {
        return sendJson(res, 400, { error: "Invalid JSON request body" });
    }

    if (!passwordMatches(body.password, process.env.DASHBOARD_PASSWORD)) {
        return sendJson(res, 401, { error: "Incorrect password" });
    }

    const token = createSessionToken();
    res.setHeader(
        "Set-Cookie",
        `${SESSION_COOKIE}=${encodeURIComponent(token)}; Max-Age=${SESSION_LIFETIME_SECONDS}; Path=/; HttpOnly; Secure; SameSite=Strict`
    );
    return sendJson(res, 200, { ok: true });
}

async function readDeviceState() {
    const config = getSupabaseConfig();
    const query = new URLSearchParams({
        select: "status,last_seen,last_command_ack",
        device_id: `eq.${config.deviceId}`,
        limit: "1"
    });
    const rows = await supabaseRequest(`dryer_state?${query}`);
    return Array.isArray(rows) && rows.length > 0 ? rows[0] : null;
}

async function handleDeviceExchange(req, res) {
    if (req.method !== "POST") {
        res.setHeader("Allow", "POST");
        return sendJson(res, 405, { error: "Method not allowed" });
    }

    if (!hasValidDeviceToken(req)) {
        return sendJson(res, 401, { error: "Invalid device token" });
    }

    if (!hasJsonContentType(req)) {
        return sendJson(res, 415, { error: "Expected application/json" });
    }

    let body;
    try {
        body = readBody(req);
    } catch {
        return sendJson(res, 400, { error: "Invalid JSON request body" });
    }

    if (!isValidTelemetry(body.status) || !isValidAcknowledgement(body.ack ?? null)) {
        return sendJson(res, 400, { error: "Invalid device update" });
    }

    const config = getSupabaseConfig();
    const command = await supabaseRequest(
        "rpc/device_exchange",
        "POST",
        {
            p_device_id: config.deviceId,
            p_status: body.status,
            p_ack: body.ack ?? null
        }
    );

    return sendJson(res, 200, { command });
}

async function handleStatus(req, res) {
    if (req.method !== "GET") {
        res.setHeader("Allow", "GET");
        return sendJson(res, 405, { error: "Method not allowed" });
    }

    const record = await readDeviceState();
    if (!record) {
        return sendJson(res, 200, {
            status: null,
            online: false,
            lastCommandAck: null
        });
    }

    return sendJson(res, 200, {
        status: record.status,
        online: isDeviceOnline(record),
        lastCommandAck: record.last_command_ack
    });
}

async function handleCommand(req, res) {
    if (req.method !== "POST") {
        res.setHeader("Allow", "POST");
        return sendJson(res, 405, { error: "Method not allowed" });
    }

    if (!hasJsonContentType(req)) {
        return sendJson(res, 415, { error: "Expected application/json" });
    }

    let body;
    try {
        body = readBody(req);
    } catch {
        return sendJson(res, 400, { error: "Invalid JSON request body" });
    }

    const action = body.action;
    const command = {
        id: crypto.randomUUID(),
        action,
        expiresAt: new Date(Date.now() + COMMAND_LIFETIME_MS).toISOString()
    };

    if (action === "settings") {
        const temperature = Number(body.temperature);
        const hours = Number(body.hours);

        if (!Number.isFinite(temperature) ||
            temperature < 30 || temperature > 80 ||
            !Number.isInteger(hours) || hours < 1 || hours > 24) {
            return sendJson(res, 400, { error: "Invalid drying settings" });
        }

        command.temperature = temperature;
        command.hours = hours;
    } else if (action !== "start" && action !== "stop") {
        return sendJson(res, 400, { error: "Unsupported command" });
    }

    const record = await readDeviceState();
    if (!isDeviceOnline(record)) {
        return sendJson(res, 409, { error: "Dryer is offline" });
    }

    const config = getSupabaseConfig();
    const queued = await supabaseRequest(
        "rpc/queue_dryer_command",
        "POST",
        {
            p_device_id: config.deviceId,
            p_command: command
        }
    );

    if (queued !== true) {
        return sendJson(res, 409, {
            error: "Dryer went offline or another command is still pending"
        });
    }

    return sendJson(res, 202, { queued: true, id: command.id });
}

async function handleApi(req, res, pathname) {
    if (pathname === "/api/session" && req.method === "GET") {
        return sendJson(res, 200, { authenticated: hasValidSession(req) });
    }

    if (pathname === "/api/login") {
        return handleLogin(req, res);
    }

    if (pathname === "/api/device/exchange") {
        return handleDeviceExchange(req, res);
    }

    if (pathname === "/api/logout" && req.method === "POST") {
        res.setHeader(
            "Set-Cookie",
            `${SESSION_COOKIE}=; Max-Age=0; Path=/; HttpOnly; Secure; SameSite=Strict`
        );
        return sendJson(res, 200, { ok: true });
    }

    if (!hasValidSession(req)) {
        return sendJson(res, 401, { error: "Sign in to use the dashboard" });
    }

    if (pathname === "/api/status") {
        return handleStatus(req, res);
    }

    if (pathname === "/api/command") {
        return handleCommand(req, res);
    }

    if (pathname === "/api/logout") {
        res.setHeader("Allow", "POST");
        return sendJson(res, 405, { error: "Method not allowed" });
    }

    return sendJson(res, 404, { error: "Not found" });
}

module.exports = async function handler(req, res) {
    try {
        const url = new URL(req.url, `https://${req.headers.host || "localhost"}`);
        const route = url.searchParams.get("route");
        const pathname = route ? `/api/${route}` : url.pathname;
        return await handleApi(req, res, pathname);
    } catch (error) {
        console.error("Dashboard API error:", error.message);
        if (error.statusCode) {
            return sendJson(res, error.statusCode, {
                error: "Cloud database is unavailable"
            });
        }
        if (error.message.includes("environment variables") ||
            error.message.includes("SESSION_SECRET") ||
            error.message.includes("DEVICE_TOKEN")) {
            return sendJson(res, 503, { error: "Dashboard service is not configured" });
        }
        return sendJson(res, 502, { error: "Dashboard service could not be reached" });
    }
};
