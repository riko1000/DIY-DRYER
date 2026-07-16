#include <Arduino.h>

#include "config.h"
#include "app.h"

App app;

void setup()
{
    Serial.begin(115200);

    analogReadResolution(14);

    app.begin();

    Serial.println();
    Serial.println("DIY Dryer Booting");
}

void loop()
{
    app.update();
}