#ifndef AMBIENT_LIGHTS_H
#define AMBIENT_LIGHTS_H

#include <Adafruit_NeoPixel.h>

#include "dryer_types.h"

class AmbientLights
{
public:
    AmbientLights();

    void begin();
    void update(DryerState state);

private:
    Adafruit_NeoPixel strip;
    DryerState lastState = DryerState::Booting;
    unsigned long lastFrameAt = 0;
    uint16_t hueOffset = 0;
    bool stateInitialized = false;

    void showIdleWhite();
    void showDryingRainbow();
    void showErrorRed();
};

#endif
