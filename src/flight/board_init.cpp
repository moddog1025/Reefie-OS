#include "board_init.h"
#include "reefie_config.h"

Barometer baro;
//IMU imu;
FlightParameters flightParams;
String eventMarker = "";

bool initializeBoard() {
    // Initialize I2C and wait a moment.
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    delay(250);

    // Set up pins.
    pinMode(PYRO_FIRE_PIN, OUTPUT);
    digitalWrite(PYRO_FIRE_PIN, LOW);
    pinMode(PYRO_CONTINUITY_PIN, INPUT);
    pinMode(LIGHT_SENSOR_PIN, INPUT);
    pinMode(BATTERY_VOLTAGE_PIN, INPUT);

    // Initialize sensors.
    baro.begin();
    //imu.begin();

    // Set flight parameters to default values.
    flightParams = {
        DEF_ACCEL_THRESHOLD,
        DEF_LIGHT_THRESHOLD,
        DEF_DISREEF_ALT,
        DEF_VELOC_THRESHOLD,
        DEF_POLL_FREQ,
        {
            true, true, true, true, true, true,
            true, true, true, true, true
        }
    };

    if (initFileSystem()) {
        String json = readFileToString("/flight_params.json");
        if (json.length() > 0) {
            StaticJsonDocument<512> doc;
            DeserializationError err = deserializeJson(doc, json);
            if (!err) {
                flightParams.ACCEL_THRESHOLD = doc["ACCEL_THRESHOLD"] | flightParams.ACCEL_THRESHOLD;
                flightParams.LIGHT_THRESHOLD = doc["LIGHT_THRESHOLD"] | flightParams.LIGHT_THRESHOLD;
                flightParams.DISREEF_ALT     = doc["DISREEF_ALT"]     | flightParams.DISREEF_ALT;
                flightParams.VELOC_THRESHOLD = doc["VELOC_THRESHOLD"] | flightParams.VELOC_THRESHOLD;
                flightParams.POLL_FREQ       = doc["POLL_FREQ"]       | flightParams.POLL_FREQ;

                if (doc.containsKey("log")) {
                    JsonObject log = doc["log"];
                    flightParams.log.altitude         = log["altitude"]         | flightParams.log.altitude;
                    flightParams.log.raw_altitude     = log["raw_altitude"]     | flightParams.log.raw_altitude;
                    flightParams.log.velocity         = log["velocity"]         | flightParams.log.velocity;
                    flightParams.log.pressure         = log["pressure"]         | flightParams.log.pressure;
                    flightParams.log.temperature      = log["temperature"]      | flightParams.log.temperature;
                    flightParams.log.accel            = log["accel"]            | flightParams.log.accel;
                    flightParams.log.gyro             = log["gyro"]             | flightParams.log.gyro;
                    flightParams.log.light            = log["light"]            | flightParams.log.light;
                    flightParams.log.continuity       = log["continuity"]       | flightParams.log.continuity;
                    flightParams.log.battery_voltage  = log["battery_voltage"]  | flightParams.log.battery_voltage;
                    flightParams.log.events           = log["events"]           | flightParams.log.events;
                }
            } else {
                Serial.print("JSON deserialization error: ");
                Serial.println(err.c_str());
            }
        } else {
            Serial.println("No flight_params.json found or file is empty.");
        }
    } else {
        Serial.println("LittleFS initialization failed!");
    }

    return true;
}
