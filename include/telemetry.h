#pragma once

#include "reefie_config.h"

struct Telemetry {
    float altitude;
    float velocity;
    float accel_mag;
    uint8_t light;
    uint8_t continuityReading;
    String currentState;
};

extern Telemetry Telem;

void updateTelemetry();