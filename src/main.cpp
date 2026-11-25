#include <Arduino.h>

#include "config.h"
#include "dryer.h"
#include "cloud_api.h"
#include "ambient_lights.h"

Dryer dryer;
CloudApi cloudApi(dryer);
AmbientLights ambientLights;

void setup()
{
    Serial.begin(115200);

    analogReadResolution(14);

    dryer.begin();
    ambientLights.begin();
    ambientLights.update(dryer.state());

    cloudApi.begin();

    Serial.println();
    Serial.println("DIY Dryer Booting");
}

void loop()
{
    dryer.update();
    ambientLights.update(dryer.state());

    cloudApi.update();
}