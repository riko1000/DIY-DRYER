#include <Arduino.h>

#include "config.h"
#include "heater.h"

void Heater::begin()
{
    pinMode(RELAY_PIN, OUTPUT);

    off();
}

void Heater::on()
{
    digitalWrite(RELAY_PIN, HIGH);

    state = true;
}

void Heater::off()
{
    digitalWrite(RELAY_PIN, LOW);

    state = false;
}

bool Heater::isOn() const
{
    return state;
}