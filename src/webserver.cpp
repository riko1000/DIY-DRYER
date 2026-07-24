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

    if (request.startsWith("GET /app.js"))
    {
        sendAppJs(client);
        return;
    }

    if (request.startsWith("GET /style.css"))
    {
        sendStyleCss(client);
        return;
    }

    if (request.startsWith("GET / "))
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

                while (contentLength > 0)
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

    client.print(R"rawliteral(
<!DOCTYPE html>
<html lang="en">

<head>

<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">

<title>DIY Dryer</title>

<link rel="stylesheet" href="/style.css">

</head>

<body>

<div class="container">

    <header>

        <h1>DIY Dryer</h1>

        <div id="statusBadge" class="status idle">
            ● IDLE
        </div>

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

        <p id="remainingTime">
            --:--:--
        </p>

    </section>

    <section class="controls">

        <button id="startButton">
            ▶ Start
        </button>

        <button id="stopButton">
            ■ Stop
        </button>

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
            <button>Custom</button>

        </div>

    </section>

    <section class="settings">

    <h2>⚙ Drying Settings</h2>

    <h3>Temperature</h3>

    <input
        id="targetTemp"
        type="range"
        min="30"
        max="80"
        value="45">

    <p><span id="targetValue">45</span> °C</p>

    <h3>Dry Time</h3>

    <input
        id="timeSlider"
        type="range"
        min="1"
        max="24"
        value="6">

    <p><span id="timeValue">6</span> Hours</p>

</section>

    <section class="graph">

        <h2>Temperature Graph</h2>

        <canvas
            id="graph"
            width="600"
            height="250">
        </canvas>

    </section>

</div>

<script src="/app.js"></script>

</body>

</html>
)rawliteral");
}
void WebServer::sendStyleCss(WiFiClient& client)
{
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/css");
    client.println("Connection: close");
    client.println();

    client.print(R"rawliteral(
* {
    margin: 0;
    padding: 0;
    box-sizing: border-box;
}

:root {
    --bg: #121212;
    --card: #1e1e1e;
    --border: #2d2d2d;
    --text: #ffffff;
    --secondary: #a1a1aa;
    --blue: #3b82f6;
    --green: #22c55e;
    --orange: #f59e0b;
    --red: #ef4444;
}

body {
    background: var(--bg);
    color: var(--text);
    font-family: Inter, Arial, sans-serif;
}

.container {
    width: min(1000px,95%);
    margin: auto;
    padding: 25px;
}

header {
    display:flex;
    justify-content:space-between;
    align-items:center;
    margin-bottom:25px;
}

header h1 {
    font-size:2.2rem;
}

.status {
    padding:10px 18px;
    border-radius:25px;
    font-weight:bold;
}

.status.idle { background:#444; }
.status.running { background:#22c55e; }
.status.error { background:#ef4444; }

.cards {
    display:grid;
    grid-template-columns:repeat(auto-fit,minmax(220px,1fr));
    gap:20px;
    margin-bottom:25px;
}

.card,
.progressSection,
.controls,
.materials,
.settings,
.graph {
    background:#1e1e1e;
    border:1px solid #2d2d2d;
    border-radius:15px;
    padding:20px;
    margin-bottom:25px;
}

.card h2 {
    color:#a1a1aa;
    margin-bottom:15px;
}

.card p {
    font-size:2rem;
    font-weight:bold;
}

.controls {
    display:flex;
    gap:20px;
}

.controls button {
    flex:1;
    border:none;
    border-radius:12px;
    padding:18px;
    color:white;
}

#startButton { background:#22c55e; }
#stopButton { background:#ef4444; }

.presetGrid {
    display:grid;
    grid-template-columns:repeat(auto-fit,minmax(120px,1fr));
    gap:15px;
    margin-top:15px;
}

.presetGrid button {
    padding:15px;
    border:none;
    border-radius:10px;
    background:#333;
    color:white;
}

input[type=range] {
    width:100%;
}

canvas {
    width:100%;
    background:#181818;
    border-radius:10px;
}
)rawliteral");
}



void WebServer::sendStatus(WiFiClient& client)
{
    JsonDocument json;

    json["state"] = dryer.getStateString();
    json["heater"] = dryer.heaterIsOn();

    json["heatbed"] = dryer.getHeatbedTemperature();
    json["chamber"] = dryer.getChamberTemperature();
    json["humidity"] = dryer.getHumidity();

    String body;

    serializeJson(json, body);

    sendJson(client, body);
}

void WebServer::sendStart(WiFiClient&)
{
}

void WebServer::sendStop(WiFiClient&)
{
}

void WebServer::sendSettings(WiFiClient& client, const String& request)
{
int bodyStart = request.indexOf("\r\n\r\n");

if (bodyStart == -1)
{
    sendText(client, "Bad Request");
    return;
}

String body = request.substring(bodyStart + 4);

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
    client.print("HTTP/1.1 ");
    client.print(statusCode);
    client.println(" OK");

    client.println("Content-Type: application/json");
    client.println("Connection: close");
    client.println();

    client.print(json);
}

void WebServer::sendHtml(WiFiClient& client,
                         const String& html,
                         int statusCode)
{
    client.print("HTTP/1.1 ");
    client.print(statusCode);
    client.println(" OK");

    client.println("Content-Type: text/html");
    client.println("Connection: close");
    client.println();

    client.print(html);
}

void WebServer::sendText(WiFiClient& client,
                         const String& text,
                         int statusCode)
{
    client.print("HTTP/1.1 ");
    client.print(statusCode);
    client.println(" OK");

    client.println("Content-Type: text/plain");
    client.println("Connection: close");
    client.println();

    client.print(text);
}
void WebServer::sendAppJs(WiFiClient& client)
{
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: application/javascript");
    client.println("Connection: close");
    client.println();

    client.print(R"rawliteral(const chamberTemp = document.getElementById("chamberTemp");
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

timeSlider.addEventListener("change", async () =>
{
    timeValue.textContent = timeSlider.value;
    await sendSettings();
});

targetSlider.addEventListener("change", async () =>
{
    targetValue.textContent = targetSlider.value;
    await sendSettings();
});

startButton.addEventListener("click", async () =>
{
    await fetch("/api/start",
    {
        method: "POST"
    });
});

stopButton.addEventListener("click", async () =>
{
    await fetch("/api/stop",
    {
        method: "POST"
    });
});

const presets = {
    PLA:  { temp: 45, hours: 6 },
    PETG: { temp: 65, hours: 6 },
    ABS:  { temp: 70, hours: 4 },
    ASA:  { temp: 75, hours: 4 },
    TPU:  { temp: 50, hours: 8 },
    PA:   { temp: 70, hours: 12 },
    PC:   { temp: 75, hours: 8 }
};


async function sendSettings()
{
    await fetch("/api/settings", {
        method: "POST",
        headers: {
            "Content-Type": "application/json"
        },
        body: JSON.stringify({
            temperature: Number(targetSlider.value),
            hours: Number(timeSlider.value)
        })
    });
}



document.querySelectorAll(".materials button").forEach(button =>
{
    
    button.addEventListener("click", async () =>    {
        
        const name = button.textContent.trim();

    if (name === "Custom")
    return;

    const preset = presets[name];

        targetSlider.value = preset.temp;
        targetValue.textContent = preset.temp;

        timeSlider.value = preset.hours;
        timeValue.textContent = preset.hours;

        await sendSettings();
    });
});

async function updateStatus()
{
    try
    {
        const response = await fetch("/api/status");

        const data = await response.json();

        chamberTemp.textContent =
            data.chamber.toFixed(1) + " °C";

        heatbedTemp.textContent =
            data.heatbed.toFixed(1) + " °C";

        humidity.textContent =
            data.humidity.toFixed(0) + " %";

        progressBar.value = data.progress;

        remainingTime.textContent =
            data.remaining;

        statusBadge.textContent =
            "● " + data.state.toUpperCase();

        statusBadge.className =
            "status " + data.state.toLowerCase();
    }
    catch(error)
    {
        statusBadge.textContent = "● OFFLINE";

        statusBadge.className = "status error";
    }
}

updateStatus();

setInterval(updateStatus, 1000);

)rawliteral");
}