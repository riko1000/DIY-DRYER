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

    uint8_t stepPhase = 0;
    bool motorOn = false;
    bool clockwise = true;
    unsigned long lastStep = 0;
    uint16_t stepIntervalUs = 2000; // ~100 RPM at 512 steps/revolution
};

#endif