#include "cloud_api.h"

#include <ArduinoJson.h>
#include <cstring>

#include "config.h"
#include "dryer.h"
#include "secrets.h"

namespace
{
constexpr unsigned long WIFI_RETRY_INTERVAL_MS = 10000;
constexpr unsigned long DEVICE_POLL_INTERVAL_MS = 10000;
constexpr unsigned long HTTP_RESPONSE_TIMEOUT_MS = 1000;
}

CloudApi::CloudApi(Dryer& dryer)
    : dryer(dryer),
      httpClient(tlsClient, VERCEL_HOST, 443)
{
}

void CloudApi::begin()
{
    Serial.println("Connecting to home Wi-Fi...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    lastWifiAttempt = millis();
    lastExchange = millis() - DEVICE_POLL_INTERVAL_MS;
    httpClient.setHttpResponseTimeout(HTTP_RESPONSE_TIMEOUT_MS);
    tlsClient.setTimeout(HTTP_RESPONSE_TIMEOUT_MS);
}

void CloudApi::update()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        if (wifiWasConnected)
        {
            Serial.println("Wi-Fi disconnected");
            wifiWasConnected = false;
        }

        const unsigned long now = millis();
        if (now - lastWifiAttempt >= WIFI_RETRY_INTERVAL_MS)
        {
            lastWifiAttempt = now;
            WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
        }
        return;
    }

    if (!wifiWasConnected)
    {
        Serial.println("Wi-Fi connected");
        wifiWasConnected = true;
    }

    const unsigned long now = millis();
    if (!pendingAckJustCreated &&
        now - lastExchange < DEVICE_POLL_INTERVAL_MS)
        return;

    lastExchange = now;
    pendingAckJustCreated = false;
    exchange();
}

void CloudApi::exchange()
{
    const DryerStatus status = dryer.getStatus();

    JsonDocument request;
    JsonObject telemetry = request["status"].to<JsonObject>();
    telemetry["state"] = dryer.getStateString();
    telemetry["heater"] = status.heaterOn;
    telemetry["motor"] = status.motorOn;
    telemetry["running"] = status.running;
    telemetry["target"] = status.targetTemperature;
    telemetry["heatbed"] = status.heatbedTemperature;
    telemetry["chamber"] = status.chamberTemperature;
    telemetry["humidity"] = status.humidity;
    telemetry["dhtConnected"] = status.dhtConnected;
    telemetry["progress"] = status.progress;
    telemetry["remainingSeconds"] = status.remainingSeconds;
    telemetry["uptimeSeconds"] = millis() / 1000;

    if (pendingAckId.length() > 0)
    {
        request["ack"]["id"] = pendingAckId;
        request["ack"]["ok"] = pendingAckAccepted;
    }
    else
    {
        request["ack"] = nullptr;
    }

    String body;
    serializeJson(request, body);

    httpClient.beginRequest();
    const int requestResult = httpClient.post(VERCEL_DEVICE_PATH);

    if (requestResult != HTTP_SUCCESS)
    {
        Serial.print("Dashboard connection failed: ");
        Serial.println(requestResult);
        apiWasConnected = false;
        httpClient.stop();
        return;
    }

    httpClient.sendHeader("Authorization", String("Bearer ") + DEVICE_TOKEN);
    httpClient.sendHeader("Content-Type", "application/json");
    httpClient.sendHeader("Content-Length", body.length());
    httpClient.beginBody();
    httpClient.print(body);
    httpClient.endRequest();

    const int responseCode = httpClient.responseStatusCode();
    const String responseBody = httpClient.responseBody();
    httpClient.stop();

    if (responseCode != 200)
    {
        Serial.print("Dashboard API returned HTTP ");
        Serial.println(responseCode);
        apiWasConnected = false;
        return;
    }

    if (!apiWasConnected)
    {
        Serial.println("Vercel API connected");
        apiWasConnected = true;
    }

    if (pendingAckId.length() > 0)
        pendingAckId = "";

    JsonDocument response;
    if (deserializeJson(response, responseBody))
    {
        Serial.println("Dashboard returned invalid JSON");
        return;
    }

    if (!response["command"].is<JsonObject>())
        return;

    JsonObject command = response["command"].as<JsonObject>();
    if (command["id"].is<const char*>())
        applyCommand(command);
}

void CloudApi::applyCommand(JsonObject command)
{
    const char* commandId = command["id"] | "";
    const char* action = command["action"] | "";
    bool accepted = false;

    if (std::strcmp(action, "start") == 0)
    {
        dryer.start();
        accepted = true;
    }
    else if (std::strcmp(action, "stop") == 0)
    {
        dryer.stop();
        accepted = true;
    }
    else if (std::strcmp(action, "settings") == 0 &&
             command["temperature"].is<float>() &&
             command["hours"].is<unsigned int>())
    {
        const float temperature = command["temperature"].as<float>();
        const uint32_t hours = command["hours"].as<uint32_t>();

        if (temperature >= 30.0f && temperature <= 80.0f &&
            hours >= 1 && hours <= 24)
        {
            dryer.setTargetTemperature(temperature);
            dryer.setDryTimeHours(hours);
            accepted = true;
        }
    }

    pendingAckId = commandId;
    pendingAckAccepted = accepted;
    pendingAckJustCreated = true;
}
