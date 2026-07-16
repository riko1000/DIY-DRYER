#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <Arduino.h>
#include <WiFiS3.h>

class Dryer;

class WebServer
{
public:
    explicit WebServer(Dryer& dryer);

    void begin();
    void update();

private:
    Dryer& dryer;

    WiFiServer server;

    void handleClient(WiFiClient& client);

    void sendHomePage(WiFiClient& client);
    void sendStatus(WiFiClient& client);

    void sendStart(WiFiClient& client);
    void sendStop(WiFiClient& client);
    void sendSettings(WiFiClient& client);

    void sendNotFound(WiFiClient& client);

    String readRequest(WiFiClient& client);

    void sendJson(
        WiFiClient& client,
        const String& json,
        int statusCode = 200);

    void sendHtml(
        WiFiClient& client,
        const String& html,
        int statusCode = 200);

    void sendText(
        WiFiClient& client,
        const String& text,
        int statusCode = 200);
};

#endif