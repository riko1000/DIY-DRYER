#include <Arduino.h>

#include "config.h"
#include "stepper.h"

void StepperMotor::begin()
{
    pinMode(STEPPER_STEP_PIN, OUTPUT);
    pinMode(STEPPER_DIR_PIN, OUTPUT);
    pinMode(STEPPER_ENABLE_PIN, OUTPUT);

    // Set initial state
    digitalWrite(STEPPER_STEP_PIN, LOW);
    digitalWrite(STEPPER_DIR_PIN, LOW);  // Direction doesn't matter for filament spinning
    digitalWrite(STEPPER_ENABLE_PIN, HIGH); // Disable motor initially

    setRPM(DEFAULT_STEPPER_RPM);

    Serial.println("[Stepper] Filament spinner initialized at " + String(DEFAULT_STEPPER_RPM) + " RPM");
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
    // Simple pulse for NEMA 17 stepping
    digitalWrite(STEPPER_STEP_PIN, HIGH);
    delayMicroseconds(5);
    digitalWrite(STEPPER_STEP_PIN, LOW);
}

void StepperMotor::on()
{
    if (!motorOn)
    {
        motorOn = true;
        digitalWrite(STEPPER_ENABLE_PIN, LOW); // Enable motor
        Serial.println("[Stepper] Filament spinner ON at " + String((60000000.0f / stepIntervalUs) / STEPS_PER_REVOLUTION) + " RPM");
    }
}

void StepperMotor::off()
{
    if (motorOn)
    {
        motorOn = false;
        digitalWrite(STEPPER_ENABLE_PIN, HIGH); // Disable motor
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
    // Direction control left in for flexibility, but not used for simple filament spinning
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


