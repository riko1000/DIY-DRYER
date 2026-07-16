#include <Arduino.h>
#include <DHT.h>

#include "config.h"
#include "dhtsensor.h"

namespace
{
    DHT dht(DHT_PIN, DHT11);
}

void DHTSensor::begin()
{
    dht.begin();
}

void DHTSensor::update()
{
    if (millis() - lastRead < 2000)
        return;

    lastRead = millis();

    float t = dht.readTemperature();
    float h = dht.readHumidity();

    if (isnan(t) || isnan(h))
    {
        sensorConnected = false;
        return;
    }

    sensorConnected = true;

    temperature = t;
    humidity = h;
}

float DHTSensor::getTemperature() const
{
    return temperature;
}

float DHTSensor::getHumidity() const
{
    return humidity;
}

bool DHTSensor::isConnected() const
{
    return sensorConnected;
}