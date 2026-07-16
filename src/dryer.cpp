#include "dryer.h"

void Dryer::begin()
{
    thermistor.begin();
    heater.begin();

    currentState = DryerState::Idle;
}

void Dryer::update()
{
    thermistor.update();
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