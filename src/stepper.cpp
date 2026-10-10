#include <Arduino.h>

#include "config.h"
#include "stepper.h"

// 28BYJ-48 half-step sequence (8 steps per full step)
// ULN2003 driver coil sequence
const uint8_t STEP_SEQUENCE[8] = {
    0b1000, // IN1
    0b1100, // IN1, IN2
    0b0100, // IN2
    0b0110, // IN2, IN3
    0b0010, // IN3
    0b0011, // IN3, IN4
    0b0001, // IN4
    0b1001  // IN4, IN1
};

void StepperMotor::begin()
{
    pinMode(STEPPER_IN1, OUTPUT);
    pinMode(STEPPER_IN2, OUTPUT);
    pinMode(STEPPER_IN3, OUTPUT);
    pinMode(STEPPER_IN4, OUTPUT);

    // Turn off all coils
    digitalWrite(STEPPER_IN1, LOW);
    digitalWrite(STEPPER_IN2, LOW);
    digitalWrite(STEPPER_IN3, LOW);
    digitalWrite(STEPPER_IN4, LOW);

    setRPM(DEFAULT_STEPPER_RPM);

    Serial.println("[Stepper] Motor initialized at " + String(DEFAULT_STEPPER_RPM) + " RPM");
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
    // Update phase based on direction
    if (clockwise)
        stepPhase = (stepPhase + 1) % 8;
    else
        stepPhase = (stepPhase - 1 + 8) % 8;

    // Set coil states based on step sequence
    uint8_t coils = STEP_SEQUENCE[stepPhase];

    digitalWrite(STEPPER_IN1, (coils & 0x08) ? HIGH : LOW);
    digitalWrite(STEPPER_IN2, (coils & 0x04) ? HIGH : LOW);
    digitalWrite(STEPPER_IN3, (coils & 0x02) ? HIGH : LOW);
    digitalWrite(STEPPER_IN4, (coils & 0x01) ? HIGH : LOW);
}

void StepperMotor::on()
{
    if (!motorOn)
    {
        motorOn = true;
        Serial.println("[Stepper] Motor ON");
    }
}

void StepperMotor::off()
{
    if (motorOn)
    {
        motorOn = false;

        // Turn off all coils to reduce power draw
        digitalWrite(STEPPER_IN1, LOW);
        digitalWrite(STEPPER_IN2, LOW);
        digitalWrite(STEPPER_IN3, LOW);
        digitalWrite(STEPPER_IN4, LOW);

        Serial.println("[Stepper] Motor OFF");
    }
}

void StepperMotor::setRPM(float rpm)
{
    // Calculate step interval in microseconds
    // RPM * steps/revolution * 60 seconds = steps/second
    // 1 second = 1,000,000 microseconds
    // stepIntervalUs = 1,000,000 / (RPM * STEPS_PER_REVOLUTION / 60)
    // stepIntervalUs = 60,000,000 / (RPM * STEPS_PER_REVOLUTION)

    if (rpm <= 0)
        rpm = 1; // Prevent division by zero

    stepIntervalUs = (uint16_t)(60000000.0f / (rpm * STEPS_PER_REVOLUTION));

    Serial.print("[Stepper] RPM set to ");
    Serial.print(rpm);
    Serial.print(" | Step interval: ");
    Serial.print(stepIntervalUs);
    Serial.println(" µs");
}

void StepperMotor::setDirection(bool cw)
{
    clockwise = cw;
    Serial.println("[Stepper] Direction: " + String(clockwise ? "CW" : "CCW"));
}

bool StepperMotor::isOn() const
{
    return motorOn;
}

bool StepperMotor::isRunning() const
{
    return motorOn;
}
