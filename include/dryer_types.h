#ifndef DRYER_TYPES_H
#define DRYER_TYPES_H

#include <Arduino.h>

enum class DryerState
{
    Booting,
    Idle,
    Heating,
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
};

struct DryerRuntime
{
    bool heaterEnabled = false;

    bool running = false;

    unsigned long startTime = 0;

    unsigned long elapsedSeconds = 0;

    unsigned long remainingSeconds = 0;
};

#endif