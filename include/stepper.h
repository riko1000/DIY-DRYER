#ifndef STEPPER_H
#define STEPPER_H

#include <Arduino.h>

class StepperMotor
{
public:
    void begin();
    void update();

    void on();
    void off();
    void setRPM(float rpm);
    void setDirection(bool clockwise);

    bool isOn() const;
    bool isRunning() const;

private:
    void step();

    bool motorOn = false;
    bool clockwise = true;
    unsigned long lastStep = 0;
    uint32_t stepIntervalUs = 5000; // ~100 RPM at 200 steps/revolution
};

#endif