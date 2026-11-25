const loginView = document.getElementById("loginView");
const dashboardView = document.getElementById("dashboardView");
const loginForm = document.getElementById("loginForm");
const loginError = document.getElementById("loginError");
const logoutButton = document.getElementById("logoutButton");
const connectionMessage = document.getElementById("connectionMessage");
const feedback = document.getElementById("feedback");
const statusBadge = document.getElementById("statusBadge");
const chamberTemp = document.getElementById("chamberTemp");
const heatbedTemp = document.getElementById("heatbedTemp");
const humidity = document.getElementById("humidity");
const progressBar = document.getElementById("progressBar");
const remainingTime = document.getElementById("remainingTime");
const targetSlider = document.getElementById("targetTemp");
const targetValue = document.getElementById("targetValue");
const timeSlider = document.getElementById("timeSlider");
const timeValue = document.getElementById("timeValue");
const startButton = document.getElementById("startButton");
const stopButton = document.getElementById("stopButton");
const canvas = document.getElementById("graph");
const context = canvas.getContext("2d");

const maxPoints = 60;
const maxChamberTarget = 48;
const heatbedHistory = [];
const chamberHistory = [];
let currentTarget = 45;
let refreshInProgress = false;
let pollingTimer = null;
let waitingCommandId = null;
let commandQueuedAt = 0;

const presets = {
    PLA: { temp: 45, hours: 6 },
    PETG: { temp: 65, hours: 6 },
    ABS: { temp: 70, hours: 4 },
    ASA: { temp: 75, hours: 4 },
    TPU: { temp: 50, hours: 8 },
    PA: { temp: 70, hours: 12 },
    PC: { temp: 75, hours: 8 }
};

function showLogin() {
    if (pollingTimer) {
        window.clearInterval(pollingTimer);
        pollingTimer = null;
    }
    loginView.hidden = false;
    dashboardView.hidden = true;
    setControlsEnabled(false);
}

function showDashboard() {
    loginView.hidden = true;
    dashboardView.hidden = false;
    setControlsEnabled(false);
    if (!pollingTimer) {
        pollingTimer = window.setInterval(refreshStatus, 5000);
    }
}

function setControlsEnabled(enabled) {
    startButton.disabled = !enabled;
    stopButton.disabled = !enabled;
    document.querySelectorAll(".materials button").forEach(button => {
        const preset = presets[button.textContent.trim()];
        const supported = preset && preset.temp <= maxChamberTarget;
        button.disabled = !enabled || !supported;
        button.title = supported
            ? ""
            : `Requires a chamber sensor rated for ${preset ? preset.temp : "this"} °C.`;
    });
    targetSlider.disabled = !enabled;
    timeSlider.disabled = !enabled;
}

function showFeedback(message, isError = false) {
    feedback.textContent = message;
    feedback.className = isError ? "feedback error" : "feedback";
    feedback.hidden = !message;
}

async function requestJson(path, options = {}) {
    const response = await fetch(path, {
        cache: "no-store",
        credentials: "same-origin",
        ...options
    });
    const body = await response.json().catch(() => ({}));

    if (response.status === 401 && path !== "/api/login") {
        showLogin();
    }

    if (!response.ok) {
        throw new Error(body.error || `Request failed (${response.status})`);
    }

    return body;
}

loginForm.addEventListener("submit", async event => {
    event.preventDefault();
    loginError.hidden = true;

    const password = document.getElementById("password").value;
    try {
        await requestJson("/api/login", {
            method: "POST",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify({ password })
        });
        loginForm.reset();
        showDashboard();
        await refreshStatus();
    } catch (error) {
        loginError.textContent = error.message;
        loginError.hidden = false;
    }
});

logoutButton.addEventListener("click", async () => {
    try {
        await requestJson("/api/logout", { method: "POST" });
    } finally {
        showLogin();
    }
});

function formatRemaining(seconds) {
    const value = Math.max(0, Number(seconds) || 0);
    const hours = Math.floor(value / 3600);
    const minutes = Math.floor((value % 3600) / 60);
    const remainder = value % 60;
    return `${hours}:${String(minutes).padStart(2, "0")}:${String(remainder).padStart(2, "0")}`;
}

function pushPoint(history, value) {
    if (!Number.isFinite(value)) return;
    history.push(value);
    if (history.length > maxPoints) history.shift();
}

function formatTemperature(value) {
    return typeof value === "number" && Number.isFinite(value)
        ? `${value.toFixed(1)} °C`
        : "N/A";
}

function drawGraph() {
    const width = canvas.width;
    const height = canvas.height;
    const padding = 30;
    const minTemp = 20;
    const maxTemp = 90;

    context.clearRect(0, 0, width, height);
    context.font = "10px sans-serif";
    context.fillStyle = "#888";
    context.strokeStyle = "#333";

    for (let temperature = 20; temperature <= 80; temperature += 20) {
        const y = height - padding -
            ((temperature - minTemp) / (maxTemp - minTemp)) * (height - 2 * padding);
        context.beginPath();
        context.moveTo(padding, y);
        context.lineTo(width - 10, y);
        context.stroke();
        context.fillText(`${temperature}°`, 4, y + 3);
    }

    const targetY = height - padding -
        ((currentTarget - minTemp) / (maxTemp - minTemp)) * (height - 2 * padding);
    context.strokeStyle = "#f59e0b";
    context.setLineDash([4, 4]);
    context.beginPath();
    context.moveTo(padding, targetY);
    context.lineTo(width - 10, targetY);
    context.stroke();
    context.setLineDash([]);

    function plotLine(points, color) {
        if (points.length < 2) return;
        context.strokeStyle = color;
        context.lineWidth = 2;
        context.beginPath();
        points.forEach((value, index) => {
            const x = padding + (index / (maxPoints - 1)) * (width - padding - 10);
            const y = height - padding -
                ((value - minTemp) / (maxTemp - minTemp)) * (height - 2 * padding);
            if (index === 0) context.moveTo(x, y);
            else context.lineTo(x, y);
        });
        context.stroke();
    }

    plotLine(heatbedHistory, "#ef4444");
    plotLine(chamberHistory, "#3b82f6");
}

function renderStatus(status, online, lastCommandAck) {
    let acknowledgementReceived = false;

    if (waitingCommandId &&
        lastCommandAck &&
        lastCommandAck.id === waitingCommandId) {
        showFeedback(lastCommandAck.ok
            ? "Dryer acknowledged the command."
            : "Dryer rejected the command.", !lastCommandAck.ok);
        waitingCommandId = null;
        acknowledgementReceived = true;
    } else if (waitingCommandId && Date.now() - commandQueuedAt > 20000) {
        showFeedback("No acknowledgement received; the command may have expired.", true);
        waitingCommandId = null;
    }

    if (!status) {
        connectionMessage.textContent = "Waiting for the dryer to connect";
        statusBadge.textContent = "● OFFLINE";
        statusBadge.className = "status error";
        setControlsEnabled(false);
        return;
    }

    connectionMessage.textContent = online
        ? "Connected to dryer"
        : "Dryer offline — readings may be out of date";
    statusBadge.textContent = `● ${online ? status.state.toUpperCase() : "OFFLINE"}`;
    statusBadge.className = `status ${online ? status.state.toLowerCase() : "error"}`;
    setControlsEnabled(online && !waitingCommandId);

    if (status.dhtConnected) {
        chamberTemp.textContent = formatTemperature(status.chamber);
        humidity.textContent = typeof status.humidity === "number" &&
            Number.isFinite(status.humidity)
            ? `${status.humidity.toFixed(0)} %`
            : "N/A";
        pushPoint(chamberHistory, status.chamber);
    } else {
        chamberTemp.textContent = "N/A";
        humidity.textContent = "N/A";
    }

    heatbedTemp.textContent = formatTemperature(status.heatbed);
    pushPoint(heatbedHistory, status.heatbed);
    progressBar.value = Number(status.progress) || 0;
    remainingTime.textContent = `${formatRemaining(status.remainingSeconds)} remaining`;

    currentTarget = Number(status.target) || currentTarget;
    targetSlider.value = currentTarget;
    targetValue.textContent = currentTarget;
    drawGraph();

    if (online && !waitingCommandId && !acknowledgementReceived) {
        showFeedback("");
    }
}

async function refreshStatus() {
    if (refreshInProgress || dashboardView.hidden) return;
    refreshInProgress = true;

    try {
        const result = await requestJson("/api/status");
        renderStatus(result.status, result.online, result.lastCommandAck);
    } catch (error) {
        if (!dashboardView.hidden) {
            connectionMessage.textContent = error.message;
            statusBadge.textContent = "● OFFLINE";
            statusBadge.className = "status error";
            setControlsEnabled(false);
        }
    } finally {
        refreshInProgress = false;
    }
}

async function sendCommand(command) {
    setControlsEnabled(false);
    showFeedback("Sending command...");
    try {
        const result = await requestJson("/api/command", {
            method: "POST",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify(command)
        });
        waitingCommandId = result.id;
        commandQueuedAt = Date.now();
        showFeedback("Command queued; waiting for the dryer to acknowledge it.");
        await refreshStatus();
    } catch (error) {
        showFeedback(error.message, true);
        await refreshStatus();
    }
}

function sendSettings() {
    return sendCommand({
        action: "settings",
        temperature: Number(targetSlider.value),
        hours: Number(timeSlider.value)
    });
}

targetSlider.addEventListener("input", () => {
    targetValue.textContent = targetSlider.value;
    currentTarget = Number(targetSlider.value);
    drawGraph();
});
targetSlider.addEventListener("change", sendSettings);

timeSlider.addEventListener("input", () => {
    timeValue.textContent = timeSlider.value;
});
timeSlider.addEventListener("change", sendSettings);

startButton.addEventListener("click", () => sendCommand({ action: "start" }));
stopButton.addEventListener("click", () => sendCommand({ action: "stop" }));

document.querySelectorAll(".materials button").forEach(button => {
    button.addEventListener("click", () => {
        const preset = presets[button.textContent.trim()];
        if (!preset) return;
        if (preset.temp > maxChamberTarget) {
            showFeedback("This preset exceeds the DHT11's 50 °C range. Upgrade the chamber sensor to use it.", true);
            return;
        }
        targetSlider.value = preset.temp;
        targetValue.textContent = preset.temp;
        timeSlider.value = preset.hours;
        timeValue.textContent = preset.hours;
        sendSettings();
    });
});

async function initialize() {
    try {
        const session = await requestJson("/api/session");
        if (!session.authenticated) {
            showLogin();
            return;
        }
        showDashboard();
        await refreshStatus();
    } catch (error) {
        showLogin();
        loginError.textContent = error.message;
        loginError.hidden = false;
    }
}

initialize();
