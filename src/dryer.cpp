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

    runtime.elapsedSeconds = 0;
    runtime.remainingSeconds = settings.dryingTimeSeconds;
    runtime.progress = 0;

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
    if (!runtime.running)
        return;

    runtime.elapsedSeconds =
        (millis() - runtime.startTime) / 1000;

    if (runtime.elapsedSeconds >= settings.dryingTimeSeconds)
    {
        runtime.remainingSeconds = 0;
        runtime.progress = 100;

        stop();

        currentState = DryerState::Finished;

        return;
    }

    runtime.remainingSeconds =
        settings.dryingTimeSeconds -
        runtime.elapsedSeconds;

    runtime.progress =
        (runtime.elapsedSeconds * 100) /
        settings.dryingTimeSeconds;
}

void Dryer::checkSafety()
{
    if (!thermistor.isConnected())
    {
        enterErrorState();
        return;
    }

    if (!dht.isConnected())
    {
        enterErrorState();
        return;
    }

    if (thermistor.getTemperature() >= settings.maxHeatbedTemperature)
    {
        enterErrorState();
        return;
    }

    if (dht.getTemperature() >= settings.maxChamberTemperature)
    {
        enterErrorState();
        return;
    }
}
void Dryer::enterErrorState()
{
    runtime.running = false;
    heater.off();
    currentState = DryerState::Error;
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
uint32_t Dryer::getRemainingSeconds() const
{
    return runtime.remainingSeconds;
}
uint8_t Dryer::getProgress() const
{
    return runtime.progress;
}


DryerStatus Dryer::getStatus() const
{
    DryerStatus status;

    status.chamberTemperature = dht.getTemperature();
    status.heatbedTemperature = thermistor.getTemperature();
    status.humidity = dht.getHumidity();

    status.targetTemperature = settings.targetTemperature;

    status.heaterOn = heater.isOn();
    status.running = runtime.running;

    status.state = currentState;

    status.remainingSeconds = runtime.remainingSeconds;
    status.progress = runtime.progress;

    return status;
}