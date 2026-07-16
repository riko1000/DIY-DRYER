#include <Arduino.h>

#include "config.h"
#include "dryer.h"

Dryer dryer;

void setup()
{
    Serial.begin(115200);

    analogReadResolution(14);

    dryer.begin();

    Serial.println();
    Serial.println("DIY Dryer Booting");
}

void loop()
{
    dryer.update();

    Serial.print("Heatbed: ");
    Serial.print(dryer.getHeatbedTemperature());
    Serial.println(" C");

    delay(1000);
}