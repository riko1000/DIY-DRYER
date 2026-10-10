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

// 28BYJ-48 Stepper Motor (ULN2003 Driver)
constexpr uint8_t STEPPER_IN1 = 8;
constexpr uint8_t STEPPER_IN2 = 9;
constexpr uint8_t STEPPER_IN3 = 10;
constexpr uint8_t STEPPER_IN4 = 11;

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
| Stepper Motor (28BYJ-48)
|--------------------------------------------------------------------------
*/

constexpr float DEFAULT_STEPPER_RPM = 10.0f;
constexpr uint16_t STEPS_PER_REVOLUTION = 2048; // 28BYJ-48 with 64:1 gearbox

/*
|--------------------------------------------------------------------------
| WiFi Access Point
|--------------------------------------------------------------------------
*/

constexpr char AP_SSID[] = "Dryer-4338";
constexpr char AP_PASSWORD[] = "43384338";

#endif