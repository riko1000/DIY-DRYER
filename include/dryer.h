#ifndef DRYER_H
#define DRYER_H

#include "heater.h"
#include "thermistor.h"
#include "dhtsensor.h"
#include "dryer_types.h"

class Dryer
{
public:
        void start();
    void stop();

    bool isRunning() const;

    void setTargetTemperature(float temperature);
    float getTargetTemperature() const;

    uint32_t getRemainingSeconds() const;
    uint8_t getProgress() const;

    Dryer() = default;

    void begin();
    void update();

    float getHeatbedTemperature() const;
    float getChamberTemperature() const;
    float getHumidity() const;
    bool heaterIsOn() const;

    DryerState state() const;
    DryerStatus getStatus() const;
private:
    
    void enterErrorState();
    void controlHeater();
    void updateTimer();
    void checkSafety();

    Thermistor thermistor;
    Heater heater;
    DHTSensor dht;

    DryerSettings settings;
    DryerRuntime runtime;

    DryerState currentState = DryerState::Booting;
};

#endif