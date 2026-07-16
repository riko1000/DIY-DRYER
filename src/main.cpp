#include <Arduino.h>

#include "config.h"
#include "thermistor.h"
#include "heater.h"

Thermistor heatbed;
Heater heater;
void setup()
{
    Serial.begin(115200);

    analogReadResolution(14);

    heater.begin();
    
    heatbed.begin();

    Serial.println();
    Serial.println("DIY Dryer Booting");
}

void loop()
{
    heatbed.update();

    Serial.print("Heatbed: ");

    Serial.print(heatbed.getTemperature());

    Serial.print(" C");

    if (!heatbed.connected())
        Serial.print("  (Sensor Missing)");

    Serial.println();

    delay(1000);
}