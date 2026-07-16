#include "dryer.h"

void Dryer::begin()
{
    thermistor.begin();
    dht.begin();
    heater.begin();

    currentState = DryerState::Idle;
}

void Dryer::start()
{
    runtime.running = true;
    runtime.startTime = millis();
    currentState = DryerState::Drying;
}

void Dryer::stop()
{
    runtime.running = false;
    heater.off();
    currentState = DryerState::Idle;
}

bool Dryer::isRunning() const
{
    return runtime.running;
}

void Dryer::setTargetTemperature(float temperature)
{
    settings.targetTemperature = temperature;
}

float Dryer::getTargetTemperature() const
{
    return settings.targetTemperature;
}

void Dryer::update()
{
    thermistor.update();
    dht.update();

    checkSafety();
    updateTimer();
    controlHeater();
}

void Dryer::controlHeater()
{
    if (!runtime.running)
    {
        heater.off();
        return;
    }

    float chamberTemp = dht.getTemperature();

    if (chamberTemp <= settings.targetTemperature - settings.hysteresis)
    {
        heater.on();
    }
    else if (chamberTemp >= settings.targetTemperature + settings.hysteresis)
    {
        heater.off();
    }
}

void Dryer::updateTimer()
{
    // TODO: Implement drying timer
}

void Dryer::checkSafety()
{
    // TODO: Implement safety checks
}

float Dryer::getHeatbedTemperature() const
{
    return thermistor.getTemperature();
}

float Dryer::getChamberTemperature() const
{
    return dht.getTemperature();
}

float Dryer::getHumidity() const
{
    return dht.getHumidity();
}

bool Dryer::heaterIsOn() const
{
    return heater.isOn();
}

DryerState Dryer::state() const
{
    return currentState;
}