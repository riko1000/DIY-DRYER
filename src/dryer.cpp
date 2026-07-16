#include "dryer.h"

void Dryer::begin()
{
    thermistor.begin();
    heater.begin();
    dht.begin();

    currentState = DryerState::Idle;
}

void Dryer::update()
{
    thermistor.update();
    dht.update();
}

float Dryer::getHeatbedTemperature() const
{
    return thermistor.getTemperature();
}

bool Dryer::heaterIsOn() const
{
    return heater.isOn();
}

DryerState Dryer::state() const
{
    return currentState;
}

float Dryer::getChamberTemperature() const
{
    return dht.getTemperature();
}

float Dryer::getHumidity() const
{
    return dht.getHumidity();
}