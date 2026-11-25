#include "ambient_lights.h"

#include "config.h"

namespace
{
constexpr unsigned long RAINBOW_FRAME_INTERVAL_MS = 40;
constexpr uint16_t HUE_STEP_PER_FRAME = 512;
}

AmbientLights::AmbientLights()
    : strip(WS2815_PIXEL_COUNT, WS2815_DATA_PIN, NEO_GRB + NEO_KHZ800)
{
}

void AmbientLights::begin()
{
    strip.begin();
    strip.clear();
    strip.show();
}

void AmbientLights::update(DryerState state)
{
    const unsigned long now = millis();
    const bool stateChanged = !stateInitialized || state != lastState;

    if (stateChanged)
    {
        lastState = state;
        stateInitialized = true;

        if (state == DryerState::Drying)
        {
            hueOffset = 0;
            lastFrameAt = now - RAINBOW_FRAME_INTERVAL_MS;
        }
        else if (state == DryerState::Error)
        {
            showErrorRed();
            return;
        }
        else
        {
            showIdleWhite();
            return;
        }
    }

    if (state == DryerState::Drying &&
        now - lastFrameAt >= RAINBOW_FRAME_INTERVAL_MS)
    {
        lastFrameAt = now;
        showDryingRainbow();
        hueOffset += HUE_STEP_PER_FRAME;
    }
}

void AmbientLights::showIdleWhite()
{
    strip.setBrightness(WS2815_IDLE_BRIGHTNESS);
    strip.fill(strip.Color(255, 255, 255));
    strip.show();
}

void AmbientLights::showDryingRainbow()
{
    strip.setBrightness(WS2815_DRYING_BRIGHTNESS);
    const uint16_t hueStep = 65535 / WS2815_PIXEL_COUNT;

    for (uint16_t pixel = 0; pixel < WS2815_PIXEL_COUNT; ++pixel)
    {
        const uint16_t hue = hueOffset + pixel * hueStep;
        strip.setPixelColor(pixel, strip.ColorHSV(hue));
    }

    strip.show();
}

void AmbientLights::showErrorRed()
{
    strip.setBrightness(WS2815_IDLE_BRIGHTNESS);
    strip.fill(strip.Color(255, 0, 0));
    strip.show();
}
