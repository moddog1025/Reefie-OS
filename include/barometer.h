#pragma once
#include <MS5x.h>

struct BarometerData {
    float pressure     = 0.0;
    float temperature  = 0.0;
    float altitude     = 0.0;
    float raw_altitude = 0.0;
    float velocity     = 0.0;
};

class Barometer {
public:
    BarometerData data;

    bool begin();
    void poll();
    void debug();

private:
    void setGroundAltitude();

    float groundAltitude = 0.0;
    float seaLevelPressure = 0.0;
    const float alpha = 0.5; // Filter coefficient for altitude
};
