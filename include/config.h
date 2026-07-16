#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

/*
|--------------------------------------------------------------------------
| Pin Configuration
|--------------------------------------------------------------------------
*/

constexpr uint8_t RELAY_PIN = 7;
constexpr uint8_t DHT_PIN = 2;
constexpr uint8_t THERMISTOR_PIN = A0;

/*
|--------------------------------------------------------------------------
| Thermistor
|--------------------------------------------------------------------------
*/

constexpr float SERIES_RESISTOR = 100000.0f;
constexpr float THERMISTOR_NOMINAL = 100000.0f;
constexpr float TEMPERATURE_NOMINAL = 25.0f;
constexpr float BETA_COEFFICIENT = 3950.0f;

/*
|--------------------------------------------------------------------------
| Dryer Defaults
|--------------------------------------------------------------------------
*/

constexpr float DEFAULT_TARGET_TEMP = 45.0f;
constexpr uint32_t DEFAULT_DRY_TIME = 6 * 60 * 60;

constexpr float HYSTERESIS = 2.0f;

constexpr float MAX_HEATBED_TEMP = 80.0f;
constexpr float MAX_CHAMBER_TEMP = 60.0f;

/*
|--------------------------------------------------------------------------
| WiFi Access Point
|--------------------------------------------------------------------------
*/

constexpr char AP_SSID[] = "DIY-Dryer";
constexpr char AP_PASSWORD[] = "filament123";

#endif