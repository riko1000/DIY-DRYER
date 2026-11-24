#include <Arduino.h>

#include "config.h"
#include "dryer.h"
#include "cloud_api.h"

Dryer dryer;
CloudApi cloudApi(dryer);

void setup()
{
    Serial.begin(115200);

    analogReadResolution(14);

    dryer.begin();

    cloudApi.begin();

    Serial.println();
    Serial.println("DIY Dryer Booting");
}

void loop()
{
    dryer.update();

    cloudApi.update();
}