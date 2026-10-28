#ifndef MOTOR_H
#define MOTOR_H

#include <Arduino.h>
#include <Servo.h>

class VictorMotor
{
public:
    void begin();
    void update();
    void on();
    void off();
    void setSpeedPercent(float percent);
    bool isOn() const;

private:
    Servo controllerSignal;
    bool motorOn = false;
    float speedPercent = 20.0f;
    uint32_t rampStartedMs = 0;
};

#endif