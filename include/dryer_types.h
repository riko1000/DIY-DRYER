#ifndef DRYER_TYPES_H
#define DRYER_TYPES_H

#include <Arduino.h>

enum class DryerState
{
    Booting,
    Idle,
    Drying,
    Finished,
    Error
};

struct DryerSettings
{
    float targetTemperature = 45.0f;
    float hysteresis = 2.0f;

    uint32_t dryingTimeSeconds = 6 * 60 * 60;

    float maxHeatbedTemperature = 80.0f;
    float maxChamberTemperature = 60.0f;

    float stepperRPM = 10.0f;
    bool stepperAutoStart = true;
};

struct DryerRuntime
{
    bool running = false;

    unsigned long startTime = 0;

    uint32_t elapsedSeconds = 0;
    uint32_t remainingSeconds = 0;

    uint8_t progress = 0;
};

struct DryerStatus
{
    float chamberTemperature = 0.0f;
    float heatbedTemperature = 0.0f;
    float humidity = 0.0f;
    bool dhtConnected = false;

    float targetTemperature = 45.0f;

    bool heaterOn = false;
    bool running = false;
    bool stepperOn = false;

    DryerState state = DryerState::Idle;

    uint32_t remainingSeconds = 0;

    uint8_t progress = 0;
};
#endif