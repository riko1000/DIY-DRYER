#include <Arduino.h>
#include <math.h>

#include "config.h"
#include "thermistor.h"

void Thermistor::begin()
{
    readSensor();
}

void Thermistor::update()
{
    if (millis() - lastRead < 200)
        return;

    readSensor();
}

void Thermistor::readSensor()
{
    lastRead = millis();

    // Oversample 16 readings to charge the ADC capacitor and filter noise
    analogRead(THERMISTOR_PIN); // dummy read
    delayMicroseconds(50);

    uint32_t adcSum = 0;
    const uint8_t SAMPLES = 16;
    for (uint8_t i = 0; i < SAMPLES; i++)
    {
        adcSum += analogRead(THERMISTOR_PIN);
        delayMicroseconds(50);
    }
    uint16_t adc = adcSum / SAMPLES;

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

    static unsigned long lastPrint = 0;
    if (millis() - lastPrint > 2000)
    {
        lastPrint = millis();
        Serial.print("[Thermistor] ADC: ");
        Serial.print(adc);
        Serial.print(" | R: ");
        Serial.print(resistance);
        Serial.print(" Ohm | Temp: ");
        Serial.print(temperature);
        Serial.println(" C");
    }
}

float Thermistor::getTemperature() const
{
    return temperature;
}

bool Thermistor::isConnected() const
{
    return sensorConnected;
}