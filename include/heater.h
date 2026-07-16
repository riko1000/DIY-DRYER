#ifndef HEATER_H
#define HEATER_H

class Heater
{
public:
    void begin();

    void on();

    void off();

    bool isOn() const;

private:
    bool state = false;
};

#endif