#ifndef DRYER_H
#define DRYER_H

#include "heater.h"
#include "thermistor.h"
#include "dhtsensor.h"

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
    float getChamberTemperature() const;
    float getHumidity() const;
    bool heaterIsOn() const;

    DryerState state() const;

private:

    Thermistor thermistor;
    Heater heater;
    DHTSensor dht;

    DryerState currentState = DryerState::Booting;
};

#endif