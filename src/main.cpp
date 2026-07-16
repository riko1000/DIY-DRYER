#include <Arduino.h>

#include "config.h"
#include "thermistor.h"

Thermistor heatbed;

void setup()
{
    Serial.begin(115200);

    analogReadResolution(14);

    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, LOW);

    heatbed.begin();

    Serial.println();
    Serial.println("========================");
    Serial.println("DIY Dryer Booting");
    Serial.println("========================");
}

void loop()
{
    Serial.print("Heatbed: ");
    Serial.print(heatbed.readTemperature());
    Serial.println(" °C");

    delay(1000);
}