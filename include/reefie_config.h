#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <LittleFS.h>
//#include "imu.h"
#include "barometer.h"
#include "board_init.h"
#include "file_manager.h"
#include "data_logging.h"

#define REEFIE_V3L
// #define REEFIE_V3_1L

#if defined(REEFIE_V3L)
  #define I2C_SDA_PIN 21
  #define I2C_SCL_PIN 20
  #define STATUS_LED_PIN 14
  #define PYRO_FIRE_PIN 10
  #define PYRO_CONTINUITY_PIN 5
  #define LIGHT_SENSOR_PIN 3
  #define BATTERY_VOLTAGE_PIN 4


#elif defined(REEFIE_V3_1L)

#else
  #error "No board version defined"
#endif