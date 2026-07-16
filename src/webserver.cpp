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

            if (request.endsWith("\r\n\r\n"))
                return request;
        }
    }

    return request;
}

void WebServer::sendHomePage(WiFiClient& client)
{
    sendHtml(
        client,
        "<html>"
        "<head><title>DIY Dryer</title></head>"
        "<body style='font-family:sans-serif;background:#111;color:white;'>"
        "<h1>DIY Dryer Online</h1>"
        "<p>The firmware is running.</p>"
        "</body>"
        "</html>");
}

void WebServer::sendStatus(WiFiClient& client)
{
    JsonDocument json;

    json["state"] = "Idle";
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

void WebServer::sendSettings(WiFiClient&)
{
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