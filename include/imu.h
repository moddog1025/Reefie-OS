#pragma once

#include <Adafruit_LSM6DSO32.h>

struct IMUData {
    float ax = 0.0;
    float ay = 0.0;
    float az = 0.0;
    float gx = 0.0;
    float gy = 0.0;
    float gz = 0.0;
};


class IMU {
public:
    IMUData data;

    bool begin();
    void poll();
    void debug();

private:
    Adafruit_LSM6DSO32 imu;

    const float accelAlpha = 0.5;
    const float gyroAlpha = 0.5;
};
