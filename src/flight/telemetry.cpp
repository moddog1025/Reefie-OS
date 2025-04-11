#include "telemetry.h"

Telemetry Telem = {0.0f, 0.0f, 0.0f, 0, 0};

void updateTelemetry() {
    baro.poll();
    imu.poll();
    
    Telem.altitude = baro.data.altitude;
    Telem.velocity = baro.data.velocity;
    Telem.accel_mag = sqrt((imu.data.ax*imu.data.ax) + (imu.data.ay*imu.data.ay) + (imu.data.az*imu.data.az));
    Telem.light = analogRead(LIGHT_SENSOR_PIN);
    Telem.continuityReading = digitalRead(PYRO_CONTINUITY_PIN);
}