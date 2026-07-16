#ifndef THERMISTOR_H
#define THERMISTOR_H

class Thermistor
{
public:

    void begin();

    void update();

    float getTemperature() const;

    bool connected() const;

private:

    float temperature = 0.0f;

    bool sensorConnected = false;

    unsigned long lastRead = 0;
};

#endif