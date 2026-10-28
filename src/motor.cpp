#include <Arduino.h>

#include "config.h"
#include "motor.h"

void VictorMotor::begin()
{
    controllerSignal.attach(VICTOR_SIGNAL_PIN);
    controllerSignal.writeMicroseconds(VICTOR_NEUTRAL_US);
    setSpeedPercent(DEFAULT_MOTOR_SPEED_PERCENT);
}

void VictorMotor::update()
{
    if (!motorOn)
        return;

    const uint32_t elapsedMs = millis() - rampStartedMs;
    const uint32_t rampDuration = MOTOR_RAMP_DURATION_MS;
    const uint16_t targetPulse = VICTOR_NEUTRAL_US +
        static_cast<uint16_t>(speedPercent * (VICTOR_FORWARD_MAX_US - VICTOR_NEUTRAL_US) / 100.0f);

    if (elapsedMs >= rampDuration)
    {
        controllerSignal.writeMicroseconds(targetPulse);
        return;
    }

    const uint16_t rampedPulse = VICTOR_NEUTRAL_US +
        static_cast<uint16_t>((static_cast<uint32_t>(targetPulse - VICTOR_NEUTRAL_US) * elapsedMs) / rampDuration);
    controllerSignal.writeMicroseconds(rampedPulse);
}

void VictorMotor::on()
{
    if (motorOn)
        return;

    motorOn = true;
    rampStartedMs = millis();
    controllerSignal.writeMicroseconds(VICTOR_NEUTRAL_US);
}

void VictorMotor::off()
{
    motorOn = false;
    controllerSignal.writeMicroseconds(VICTOR_NEUTRAL_US);
}

void VictorMotor::setSpeedPercent(float percent)
{
    speedPercent = constrain(percent, 0.0f, 100.0f);
}

bool VictorMotor::isOn() const
{
    return motorOn;
}