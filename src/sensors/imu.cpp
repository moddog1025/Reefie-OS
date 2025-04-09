#include "imu.h"
#include "reefie_config.h"

bool IMU::begin() {
    if (!imu.begin_I2C(0x6B)) {
        Serial.println("IMU error");
        return false;
    }

    Serial.println("IMU initialized");
    return true;
}

void IMU::poll() {
    sensors_event_t accel;
    sensors_event_t gyro;
    sensors_event_t temp;
    imu.getEvent(&accel, &gyro, &temp);

    data.ax = accelAlpha * accel.acceleration.x + (1 - accelAlpha) * data.ax;
    data.ay = accelAlpha * accel.acceleration.y + (1 - accelAlpha) * data.ay;
    data.az = accelAlpha * accel.acceleration.z + (1 - accelAlpha) * data.az;

    data.gx = gyroAlpha * gyro.gyro.x + (1 - gyroAlpha) * data.gx;
    data.gy = gyroAlpha * gyro.gyro.y + (1 - gyroAlpha) * data.gy;
    data.gz = gyroAlpha * gyro.gyro.z + (1 - gyroAlpha) * data.gz;
}

void IMU::debug() {
    poll();
    Serial.print("Accel (XYZ): ");
    Serial.print(data.ax); Serial.print(", ");
    Serial.print(data.ay); Serial.print(", ");
    Serial.println(data.az);

    Serial.print("Gyro  (XYZ): ");
    Serial.print(data.gx); Serial.print(", ");
    Serial.print(data.gy); Serial.print(", ");
    Serial.println(data.gz);
}
