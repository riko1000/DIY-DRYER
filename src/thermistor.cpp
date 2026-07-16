#include <Arduino.h>
#include <math.h>

#include "config.h"
#include "thermistor.h"

void Thermistor::begin()
{
}

void Thermistor::update()
{
    if (millis() - lastRead < 200)
        return;

    lastRead = millis();

    uint16_t adc = analogRead(THERMISTOR_PIN);

    if (adc == 0 || adc >= 16383)
    {
        sensorConnected = false;
        return;
    }

    sensorConnected = true;

    float resistance =
        SERIES_RESISTOR /
        ((16383.0f / adc) - 1.0f);

    float t;

    t = resistance / THERMISTOR_NOMINAL;
    t = log(t);
    t /= BETA_COEFFICIENT;
    t += 1.0f / (TEMPERATURE_NOMINAL + 273.15f);
    t = 1.0f / t;
    t -= 273.15f;

    temperature = t;
}

float Thermistor::getTemperature() const
{
    return temperature;
}

bool Thermistor::connected() const
{
    return sensorConnected;
}