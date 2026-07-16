#ifndef DHTSENSOR_H
#define DHTSENSOR_H

class DHTSensor
{
public:
    void begin();
    void update();

    float getTemperature() const;
    float getHumidity() const;

    bool isConnected() const;

private:
    float temperature = 0.0f;
    float humidity = 0.0f;

    bool sensorConnected = false;

    unsigned long lastRead = 0;
};

#endif