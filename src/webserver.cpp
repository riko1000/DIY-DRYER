#include "webserver.h"

#include <ArduinoJson.h>

#include "config.h"
#include "dryer.h"

WebServer::WebServer(Dryer& dryer)
    : dryer(dryer),
      server(80)
{
}

void WebServer::begin()
{
    Serial.println();
    Serial.println("=================================");
    Serial.println("Starting WiFi Access Point...");
    Serial.println("=================================");

    WiFi.beginAP(AP_SSID, AP_PASSWORD);

    IPAddress ip = WiFi.localIP();

    Serial.print("SSID: ");
    Serial.println(AP_SSID);

    Serial.print("Password: ");
    Serial.println(AP_PASSWORD);

    Serial.print("IP Address: ");
    Serial.println(ip);

    server.begin();

    Serial.println("HTTP Server Started");
}

void WebServer::update()
{
    WiFiClient client = server.available();

    if (!client)
        return;

    handleClient(client);

    client.stop();
}

void WebServer::handleClient(WiFiClient& client)
{
    String request = readRequest(client);

    if (request.startsWith("GET / ") ||
        request.startsWith("GET /index.html") ||
        request.startsWith("GET /app.js") ||
        request.startsWith("GET /style.css"))
    {
        sendHomePage(client);
        return;
    }

    if (request.startsWith("GET /api/status"))
    {
        sendStatus(client);
        return;
    }

    if (request.startsWith("POST /api/start"))
    {
        dryer.start();
        sendText(client, "OK");
        return;
    }

    if (request.startsWith("POST /api/stop"))
    {
        dryer.stop();
        sendText(client, "OK");
        return;
    }

    if (request.startsWith("POST /api/settings"))
    {
        sendSettings(client, request);
        return;
    }

    sendNotFound(client);
}

String WebServer::readRequest(WiFiClient& client)
{
    String request;

    unsigned long timeout = millis();

    while (client.connected() && millis() - timeout < 1000)
    {
        while (client.available())
        {
            char c = client.read();

            request += c;

            timeout = millis();

            if (request.endsWith("\r\n\r\n"))
            {
                int contentLength = 0;

                int index = request.indexOf("Content-Length:");

                if (index != -1)
                {
                    int end = request.indexOf("\r\n", index);

                    String value = request.substring(index + 15, end);

                    value.trim();

                    contentLength = value.toInt();
                }

                while (contentLength > 0 &&
                       client.connected() &&
                       millis() - timeout < 1000)
                {
                    if (client.available())
                    {
                        request += (char)client.read();
                        contentLength--;

                        timeout = millis();
                    }
                }

                return request;
            }
        }
    }

    return request;
}

void WebServer::sendHomePage(WiFiClient& client)
{
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close");
    client.println();

    client.print(R"rawliteral(<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>4338 Dryer</title>
<style>
* { margin:0; padding:0; box-sizing:border-box; }
:root {
    --bg:#121212; --card:#1e1e1e; --border:#2d2d2d;
    --text:#ffffff; --secondary:#a1a1aa; --blue:#3b82f6;
    --green:#22c55e; --orange:#f59e0b; --red:#ef4444;
}
body { background:var(--bg); color:var(--text); font-family:Inter,Arial,sans-serif; }
.container { width:min(1000px,95%); margin:auto; padding:25px; }
header { display:flex; justify-content:space-between; align-items:center; margin-bottom:25px; }
header h1 { font-size:2.2rem; }
.status { padding:10px 18px; border-radius:25px; font-weight:bold; }
.status.idle { background:#444; }
.status.drying { background:#22c55e; }
.status.finished { background:#3b82f6; }
.status.error { background:#ef4444; }
.status.booting { background:#f59e0b; }
.cards { display:grid; grid-template-columns:repeat(auto-fit,minmax(220px,1fr)); gap:20px; margin-bottom:25px; }
.card, .progressSection, .controls, .materials, .settings, .graph {
    background:#1e1e1e; border:1px solid #2d2d2d; border-radius:15px; padding:20px; margin-bottom:25px;
}
.card h2 { color:#a1a1aa; margin-bottom:15px; }
.card p { font-size:2rem; font-weight:bold; }
.controls { display:flex; gap:20px; }
.controls button {
    flex:1; border:none; border-radius:12px; padding:18px; color:white; font-size:1.1rem; font-weight:bold; cursor:pointer;
}
#startButton { background:#22c55e; }
#stopButton { background:#ef4444; }
.controls button:hover { opacity:0.9; }
.presetGrid { display:grid; grid-template-columns:repeat(auto-fit,minmax(100px,1fr)); gap:15px; margin-top:15px; }
.presetGrid button {
    padding:15px; border:none; border-radius:10px; background:#333; color:white; font-weight:bold; cursor:pointer;
}
.presetGrid button:hover { background:#444; }
input[type=range] { width:100%; cursor:pointer; }
progress { width:100%; height:20px; border-radius:10px; overflow:hidden; }
progress::-webkit-progress-bar { background:#333; border-radius:10px; }
progress::-webkit-progress-value { background:#22c55e; border-radius:10px; }
canvas { width:100%; background:#181818; border-radius:10px; display:block; }
.legend { display:flex; gap:20px; margin-top:10px; font-size:0.9rem; color:#a1a1aa; }
.legend-item { display:flex; align-items:center; gap:6px; }
.legend-dot { width:12px; height:12px; border-radius:50%; display:inline-block; }
</style>
</head>
<body>
<div class="container">
    <header>
        <h1>4338 Dryer</h1>
        <div id="statusBadge" class="status idle">● IDLE</div>
    </header>

    <section class="cards">
        <div class="card">
            <h2>🌡 Chamber</h2>
            <p id="chamberTemp">--.- °C</p>
        </div>
        <div class="card">
            <h2>🔥 Heatbed</h2>
            <p id="heatbedTemp">--.- °C</p>
        </div>
        <div class="card">
            <h2>💧 Humidity</h2>
            <p id="humidity">-- %</p>
        </div>
    </section>

    <section class="progressSection">
        <h2>Progress</h2>
        <progress id="progressBar" value="0" max="100"></progress>
        <p id="remainingTime" style="margin-top:10px;font-size:1.5rem;font-weight:bold;">--:--:--</p>
    </section>

    <section class="controls">
        <button id="startButton">▶ Start</button>
        <button id="stopButton">■ Stop</button>
    </section>

    <section class="materials">
        <h2>Material Presets</h2>
        <div class="presetGrid">
            <button>PLA</button>
            <button>PETG</button>
            <button>ABS</button>
            <button>ASA</button>
            <button>TPU</button>
            <button>PA</button>
            <button>PC</button>
        </div>
    </section>

    <section class="settings">
        <h2>⚙ Drying Settings</h2>
        <h3>Temperature</h3>
        <input id="targetTemp" type="range" min="30" max="80" value="45">
        <p><span id="targetValue">45</span> °C</p>
        <h3 style="margin-top:15px;">Dry Time</h3>
        <input id="timeSlider" type="range" min="1" max="24" value="6">
        <p><span id="timeValue">6</span> Hours</p>
    </section>

    <section class="graph">
        <h2>Temperature Graph</h2>
        <canvas id="graph" width="600" height="200"></canvas>
        <div class="legend">
            <div class="legend-item"><span class="legend-dot" style="background:#ef4444;"></span> Heatbed</div>
            <div class="legend-item"><span class="legend-dot" style="background:#3b82f6;"></span> Chamber</div>
            <div class="legend-item"><span class="legend-dot" style="background:#f59e0b;"></span> Target</div>
        </div>
    </section>
</div>

<script>
const chamberTemp = document.getElementById("chamberTemp");
const heatbedTemp = document.getElementById("heatbedTemp");
const humidity = document.getElementById("humidity");
const statusBadge = document.getElementById("statusBadge");
const progressBar = document.getElementById("progressBar");
const remainingTime = document.getElementById("remainingTime");
const targetSlider = document.getElementById("targetTemp");
const targetValue = document.getElementById("targetValue");
const startButton = document.getElementById("startButton");
const stopButton = document.getElementById("stopButton");
const timeSlider = document.getElementById("timeSlider");
const timeValue = document.getElementById("timeValue");
const canvas = document.getElementById("graph");
const ctx = canvas.getContext("2d");

const maxPoints = 60;
const heatbedHistory = [];
const chamberHistory = [];
let currentTarget = 45;

timeSlider.addEventListener("input", () => { timeValue.textContent = timeSlider.value; });
timeSlider.addEventListener("change", async () => { timeValue.textContent = timeSlider.value; await sendSettings(); });
targetSlider.addEventListener("input", () => { targetValue.textContent = targetSlider.value; });
targetSlider.addEventListener("change", async () => { targetValue.textContent = targetSlider.value; await sendSettings(); });

startButton.addEventListener("click", async () => { await fetch("/api/start", { method: "POST" }); });
stopButton.addEventListener("click", async () => { await fetch("/api/stop", { method: "POST" }); });

const presets = {
    PLA:  { temp: 45, hours: 6 },
    PETG: { temp: 65, hours: 6 },
    ABS:  { temp: 70, hours: 4 },
    ASA:  { temp: 75, hours: 4 },
    TPU:  { temp: 50, hours: 8 },
    PA:   { temp: 70, hours: 12 },
    PC:   { temp: 75, hours: 8 }
};

async function sendSettings() {
    await fetch("/api/settings", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({
            temperature: Number(targetSlider.value),
            hours: Number(timeSlider.value)
        })
    });
}

document.querySelectorAll(".materials button").forEach(button => {
    button.addEventListener("click", async () => {
        const name = button.textContent.trim();
        const preset = presets[name];
        if (!preset) return;
        targetSlider.value = preset.temp;
        targetValue.textContent = preset.temp;
        timeSlider.value = preset.hours;
        timeValue.textContent = preset.hours;
        await sendSettings();
    });
});

function drawGraph() {
    ctx.clearRect(0, 0, canvas.width, canvas.height);
    const w = canvas.width, h = canvas.height;
    const pad = 30;
    const minT = 20, maxT = 90;

    ctx.strokeStyle = "#2d2d2d";
    ctx.lineWidth = 1;
    ctx.font = "10px sans-serif";
    ctx.fillStyle = "#666";
    for (let t = 20; t <= 90; t += 20) {
        const y = h - pad - ((t - minT) / (maxT - minT)) * (h - 2 * pad);
        ctx.beginPath();
        ctx.moveTo(pad, y);
        ctx.lineTo(w - 10, y);
        ctx.stroke();
        ctx.fillText(t + "°", 5, y + 3);
    }

    const targetY = h - pad - ((currentTarget - minT) / (maxT - minT)) * (h - 2 * pad);
    ctx.strokeStyle = "#f59e0b";
    ctx.setLineDash([4, 4]);
    ctx.beginPath();
    ctx.moveTo(pad, targetY);
    ctx.lineTo(w - 10, targetY);
    ctx.stroke();
    ctx.setLineDash([]);

    function plotLine(data, color) {
        if (data.length < 2) return;
        ctx.strokeStyle = color;
        ctx.lineWidth = 2;
        ctx.beginPath();
        data.forEach((val, i) => {
            const x = pad + (i / (maxPoints - 1)) * (w - pad - 10);
            const y = h - pad - ((val - minT) / (maxT - minT)) * (h - 2 * pad);
            if (i === 0) ctx.moveTo(x, y);
            else ctx.lineTo(x, y);
        });
        ctx.stroke();
    }

    plotLine(heatbedHistory, "#ef4444");
    plotLine(chamberHistory, "#3b82f6");
}

async function updateStatus() {
    try {
        const response = await fetch("/api/status");
        const data = await response.json();

        if (data.dhtConnected) {
            chamberTemp.textContent = data.chamber.toFixed(1) + " °C";
            humidity.textContent = data.humidity.toFixed(0) + " %";
            chamberHistory.push(data.chamber);
            if (chamberHistory.length > maxPoints) chamberHistory.shift();
        } else {
            chamberTemp.textContent = "N/A";
            humidity.textContent = "N/A";
        }

        heatbedTemp.textContent = data.heatbed.toFixed(1) + " °C";
        heatbedHistory.push(data.heatbed);
        if (heatbedHistory.length > maxPoints) heatbedHistory.shift();

        currentTarget = data.target;
        progressBar.value = data.progress;
        remainingTime.textContent = data.remaining;

        statusBadge.textContent = "● " + data.state.toUpperCase();
        statusBadge.className = "status " + data.state.toLowerCase();

        drawGraph();
    } catch(e) {
        statusBadge.textContent = "● OFFLINE";
        statusBadge.className = "status error";
    }
}

updateStatus();
setInterval(updateStatus, 1000);
</script>
</body>
</html>
)rawliteral");
}

void WebServer::sendStatusLine(WiFiClient& client, int statusCode)
{
    client.print("HTTP/1.1 ");
    client.print(statusCode);

    switch (statusCode)
    {
    case 200:
        client.println(" OK");
        break;
    case 400:
        client.println(" Bad Request");
        break;
    case 404:
        client.println(" Not Found");
        break;
    default:
        client.println(" OK");
        break;
    }
}

void WebServer::sendStatus(WiFiClient& client)
{
    JsonDocument json;
    const DryerStatus status = dryer.getStatus();

    json["state"] = dryer.getStateString();
    json["heater"] = status.heaterOn;
    json["running"] = status.running;
    json["target"] = status.targetTemperature;
    json["heatbed"] = status.heatbedTemperature;
    json["chamber"] = status.chamberTemperature;
    json["humidity"] = status.humidity;
    json["dhtConnected"] = status.dhtConnected;
    json["progress"] = status.progress;

    // format remaining seconds as H:MM:SS for the dashboard
    uint32_t remSeconds = status.remainingSeconds;
    uint32_t hours = remSeconds / 3600;
    uint32_t minutes = (remSeconds % 3600) / 60;
    uint32_t seconds = remSeconds % 60;

    char remBuf[16];
    snprintf(remBuf, sizeof(remBuf), "%u:%02u:%02u", (unsigned)hours, (unsigned)minutes, (unsigned)seconds);

    json["remaining"] = remBuf;

    String body;

    serializeJson(json, body);

    sendJson(client, body);
}

void WebServer::sendSettings(WiFiClient& client, const String& request)
{
    int bodyStart = request.indexOf("\r\n\r\n");

    if (bodyStart == -1)
    {
        sendText(client, "Bad Request", 400);
        return;
    }

    String body = request.substring(bodyStart + 4);

    JsonDocument json;
    DeserializationError error = deserializeJson(json, body);

    if (error)
    {
        sendText(client, "Bad Request", 400);
        return;
    }

    if (json["temperature"].is<float>())
    {
        dryer.setTargetTemperature(json["temperature"].as<float>());
    }

    if (json["hours"].is<unsigned int>())
    {
        dryer.setDryTimeHours(json["hours"].as<uint32_t>());
    }

    Serial.println("Settings received");
    sendText(client, "OK");
}

void WebServer::sendNotFound(WiFiClient& client)
{
    sendText(client, "404 Not Found", 404);
}

void WebServer::sendJson(WiFiClient& client,
                         const String& json,
                         int statusCode)
{
    sendStatusLine(client, statusCode);

    client.println("Content-Type: application/json");
    client.println("Connection: close");
    client.println();

    client.print(json);
}

void WebServer::sendHtml(WiFiClient& client,
                         const String& html,
                         int statusCode)
{
    sendStatusLine(client, statusCode);

    client.println("Content-Type: text/html");
    client.println("Connection: close");
    client.println();

    client.print(html);
}

void WebServer::sendText(WiFiClient& client,
                         const String& text,
                         int statusCode)
{
    sendStatusLine(client, statusCode);

    client.println("Content-Type: text/plain");
    client.println("Connection: close");
    client.println();

    client.print(text);
}