#include <Arduino.h>
#include <math.h>

#include "config.h"
#include "thermistor.h"

void Thermistor::begin()
{
    // Nothing to initialize yet.
}

float Thermistor::readTemperature()
{
    // Read the ADC (0–16383 because we set 14-bit resolution)
    uint16_t adc = analogRead(THERMISTOR_PIN);

    // Prevent divide-by-zero
    if (adc == 0 || adc >= 16383)
        return -999.0f;

    // Calculate thermistor resistance
    float resistance = SERIES_RESISTOR / ((16383.0f / adc) - 1.0f);

    // Beta equation
    float temperature;

    temperature = resistance / THERMISTOR_NOMINAL;
    temperature = log(temperature);
    temperature /= BETA_COEFFICIENT;
    temperature += 1.0f / (TEMPERATURE_NOMINAL + 273.15f);
    temperature = 1.0f / temperature;
    temperature -= 273.15f;

    return temperature;
}