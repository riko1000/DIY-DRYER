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
constexpr bool DHT_SENSOR_ENABLED = true;
constexpr uint8_t WS2815_DATA_PIN = 4;
constexpr uint16_t WS2815_PIXEL_COUNT = 100;
constexpr uint8_t WS2815_IDLE_BRIGHTNESS = 18;
constexpr uint8_t WS2815_DRYING_BRIGHTNESS = 48;

// AndyMark am-2235a brushed DC motor with VEX Victor SP
constexpr uint8_t VICTOR_SIGNAL_PIN = 8;
constexpr uint16_t VICTOR_NEUTRAL_US = 1500;
constexpr uint16_t VICTOR_FORWARD_MAX_US = 2000;

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

constexpr float DHT11_MAX_TEMP = 50.0f;
constexpr float MAX_CHAMBER_TARGET_TEMP = DHT11_MAX_TEMP - HYSTERESIS;
constexpr float MAX_HEATBED_TEMP = 80.0f;
constexpr float MAX_CHAMBER_TEMP = 60.0f;

/*
|--------------------------------------------------------------------------
| AndyMark Snow Blower DC Motor + Victor SP
|--------------------------------------------------------------------------
*/

constexpr float DEFAULT_MOTOR_SPEED_PERCENT = 30.0f;
constexpr uint16_t MOTOR_RAMP_DURATION_MS = 1500;

#endif