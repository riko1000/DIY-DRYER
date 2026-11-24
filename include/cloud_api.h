#ifndef CLOUD_API_H
#define CLOUD_API_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ArduinoHttpClient.h>
#include <WiFiS3.h>

class Dryer;

class CloudApi
{
public:
    explicit CloudApi(Dryer& dryer);

    void begin();
    void update();

private:
    Dryer& dryer;
    WiFiSSLClient tlsClient;
    HttpClient httpClient;

    unsigned long lastWifiAttempt = 0;
    unsigned long lastExchange = 0;
    bool wifiWasConnected = false;
    bool apiWasConnected = false;
    bool pendingAckJustCreated = false;
    String pendingAckId;
    bool pendingAckAccepted = false;

    void exchange();
    void applyCommand(JsonObject command);
};

#endif
