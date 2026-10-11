#include <Arduino.h>

#include "config.h"
#include "stepper.h"

void StepperMotor::begin()
{
    pinMode(STEPPER_STEP_PIN, OUTPUT);
    pinMode(STEPPER_DIR_PIN, OUTPUT);

    // Set initial state. A4988 ENABLE is hard-wired to GND.
    digitalWrite(STEPPER_STEP_PIN, LOW);
    digitalWrite(STEPPER_DIR_PIN, LOW);

    setRPM(DEFAULT_STEPPER_RPM);

    Serial.println("[Stepper] NEMA 17 motor initialized at " + String(DEFAULT_STEPPER_RPM) + " RPM");
}

void StepperMotor::update()
{
    if (!motorOn)
        return;

    unsigned long now = micros();
    if (now - lastStep >= stepIntervalUs)
    {
        lastStep = now;
        step();
    }
}

void StepperMotor::step()
{
    digitalWrite(STEPPER_STEP_PIN, HIGH);
    delayMicroseconds(5);
    digitalWrite(STEPPER_STEP_PIN, LOW);
}

void StepperMotor::on()
{
    if (!motorOn)
    {
        motorOn = true;
        Serial.println("[Stepper] Filament spinner ON");
    }
}

void StepperMotor::off()
{
    if (motorOn)
    {
        motorOn = false;
        Serial.println("[Stepper] Filament spinner OFF");
    }
}

void StepperMotor::setRPM(float rpm)
{
    if (rpm <= 0)
        rpm = 1;

    stepIntervalUs = (uint32_t)(60000000.0f / (rpm * STEPS_PER_REVOLUTION));
}

void StepperMotor::setDirection(bool cw)
{
    clockwise = cw;
}

bool StepperMotor::isOn() const
{
    return motorOn;
}

bool StepperMotor::isRunning() const
{
    return motorOn;
}


