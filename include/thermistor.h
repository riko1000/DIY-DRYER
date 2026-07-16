#ifndef THERMISTOR_H
#define THERMISTOR_H

class Thermistor
{
public:
    void begin();
    float readTemperature();
};

#endif