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
constexpr bool DHT_SENSOR_ENABLED = false;

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

constexpr float MAX_HEATBED_TEMP = 80.0f;
constexpr float MAX_CHAMBER_TEMP = 60.0f;

/*
|--------------------------------------------------------------------------
| AndyMark Snow Blower DC Motor + Victor SP
|--------------------------------------------------------------------------
*/

constexpr float DEFAULT_MOTOR_SPEED_PERCENT = 30.0f;
constexpr uint16_t MOTOR_RAMP_DURATION_MS = 1500;

/*
|--------------------------------------------------------------------------
| WiFi Access Point
|--------------------------------------------------------------------------
*/

constexpr char AP_SSID[] = "Dryer-4338";
constexpr char AP_PASSWORD[] = "43384338";

#endif