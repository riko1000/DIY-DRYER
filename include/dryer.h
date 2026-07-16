#ifndef DRYER_H
#define DRYER_H

#include "heater.h"
#include "thermistor.h"

enum class DryerState
{
    Booting,
    Idle,
    Heating,
    Finished,
    Error
};

class Dryer
{
public:
    Dryer() = default;

    void begin();
    void update();

    float getHeatbedTemperature() const;

    bool heaterIsOn() const;

    DryerState state() const;

private:

    Thermistor thermistor;
    Heater heater;

    DryerState currentState = DryerState::Booting;
};

#endif