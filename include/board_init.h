#pragma once

#include "reefie_config.h"

//Default flight parameters
#define DEF_ACCEL_THRESHOLD 30.0
#define DEF_LIGHT_THRESHOLD 50
#define DEF_DISREEF_ALT 365.0
#define DEF_VELOC_THRESHOLD 2.0
#define DEF_LAUNCH_SITE_ID 0
#define DEF_POLL_FREQ 20

struct FlightLogOptions {
  bool altitude;
  bool raw_altitude;
  bool velocity;
  bool pressure;
  bool temperature;
  bool accel;
  bool gyro;
  bool light;
  bool continuity;
  bool battery_voltage;
  bool events;
};

struct FlightParameters {
  float ACCEL_THRESHOLD;
  uint8_t LIGHT_THRESHOLD;
  float DISREEF_ALT;
  float VELOC_THRESHOLD;
  uint8_t POLL_FREQ;
  FlightLogOptions log;
};

extern FlightParameters flightParams;

extern Barometer baro;
extern IMU imu;

extern String eventMarker;

bool initializeBoard();
