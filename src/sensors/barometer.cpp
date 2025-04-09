#include "barometer.h"
#include "reefie_config.h"

MS5x MS5_Baro(&Wire);

bool Barometer::begin() {
    for (int attempt = 1; attempt <= 5; attempt++) {
        if (MS5_Baro.connect() == 0) {
            Serial.println("Barometer initialized");
            MS5_Baro.setDelay(50);

            // Wait until the sensor produces a valid pressure reading.
            unsigned long startTime = millis();
            while (!MS5_Baro.isReady() && (millis() - startTime < 2000)) {
                MS5_Baro.checkUpdates();
                delay(50);
            }
            if (!MS5_Baro.isReady()) {
                Serial.println("Barometer not ready after connection");
                return false;
            }

            // Now that _PRESS is valid, set sea level pressure
            seaLevelPressure = MS5_Baro.getSeaLevel(217.3);
            setGroundAltitude();
            return true;
        }
        Serial.println("Barometer error");
        delay(250);
    }

    Serial.println("Barometer initialization failed");
    return false;
}

void Barometer::poll() {
    static float lastAltitude = 0.0;
    static bool firstRun = true;
    static unsigned long lastMicros = micros();
    static const float velocityAlpha = 0.4;

    MS5_Baro.checkUpdates();
    if (MS5_Baro.isReady()) {
        data.temperature = MS5_Baro.GetTemp();
        data.pressure = MS5_Baro.GetPres();
        
        float currentAltitude = MS5_Baro.getAltitude(false);
        // Ensure the altitude reading is valid before using it
        if (!isfinite(currentAltitude)) {
            Serial.println("Invalid altitude reading");
            return;
        }
        data.raw_altitude = currentAltitude - groundAltitude;
        
        // On the first valid reading, initialize the filtered altitude and velocity.
        if (firstRun) {
            data.altitude = data.raw_altitude;
            data.velocity = 0;
            firstRun = false;
        } else {
            data.altitude = alpha * data.raw_altitude + (1 - alpha) * data.altitude;
        }

        unsigned long now = micros();
        float dt = (now - lastMicros) / 1e6;
        lastMicros = now;

        if (dt > 0.005) { // Avoid division by very small dt values.
            float v = (data.altitude - lastAltitude) / dt;
            data.velocity = velocityAlpha * v + (1 - velocityAlpha) * data.velocity;
            lastAltitude = data.altitude;
        }
    }
}

void Barometer::setGroundAltitude() {
    const int maxReadings = 200;
    const int minReadings = 50;
    const float stabilityThreshold = 0.05;

    float totalAltitude = 0.0;
    float lastAltitude = 0.0;
    int readingCount = 0;

    while (readingCount < maxReadings) {
        MS5_Baro.checkUpdates();
        if (MS5_Baro.isReady()) {
            float temperature = MS5_Baro.GetTemp();
            float pressure = MS5_Baro.GetPres();

            float currentAltitude = MS5_Baro.getAltitude();
            totalAltitude += currentAltitude;
            readingCount++;

            if (readingCount >= minReadings &&
                abs(currentAltitude - lastAltitude) < stabilityThreshold) {
                break;
            }

            lastAltitude = currentAltitude;
        }
        delay(25);
    }

    if (readingCount > 0) {
        groundAltitude = totalAltitude / readingCount;
    } else {
        groundAltitude = 0;
    }
}

void Barometer::debug() {
    poll();
    Serial.print("Baro Altitude: ");      Serial.println(data.altitude);
    Serial.print("Raw Altitude: ");       Serial.println(data.raw_altitude);
    Serial.print("Velocity: ");           Serial.println(data.velocity);
    Serial.print("Pressure: ");           Serial.println(data.pressure);
    Serial.print("Temperature: ");        Serial.println(data.temperature);
    delay(250);
}
