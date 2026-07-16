#include <Arduino.h>

#include "config.h"
#include "dryer.h"
#include "webserver.h"

Dryer dryer;
WebServer webServer(dryer);

void setup()
{
    Serial.begin(115200);

    analogReadResolution(14);

    dryer.begin();

    // Temporary for testing.
    dryer.start();

    webServer.begin();

    Serial.println();
    Serial.println("DIY Dryer Booting");
}

void loop()
{
    dryer.update();

    webServer.update();
}