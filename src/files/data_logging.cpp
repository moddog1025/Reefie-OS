#include "data_logging.h"
#include "reefie_config.h"

File logFile;
bool loggingActive = false;
String currentLogFilename;
unsigned long loggingStartTime = 0;

bool initLogFile() {
    
    File root = LittleFS.open("/");
    int maxFlightNum = 0;
    
    
    while (File file = root.openNextFile()) {
        String name = file.name();
        file.close();
        if (name.endsWith(".csv")) {
            int numIdx = name.lastIndexOf('_') + 1;
            int dotIdx = name.lastIndexOf('.');
            if (numIdx > 0 && dotIdx > numIdx) {
                int n = name.substring(numIdx, dotIdx).toInt();
                maxFlightNum = max(maxFlightNum, n);
            }
        }
    }
    
    
    currentLogFilename = "/jack_data_" + String(maxFlightNum + 1) + ".csv";
    
    
    File headerFile = LittleFS.open(currentLogFilename, "w");
    if (!headerFile) {
        Serial.println("Failed to open log file for header writing!");
        return false;
    }
    
    
    String header = "time";
    if (flightParams.log.altitude)        header += ",altitude";
    if (flightParams.log.raw_altitude)    header += ",raw_altitude";
    if (flightParams.log.velocity)        header += ",velocity";
    if (flightParams.log.pressure)        header += ",pressure";
    if (flightParams.log.temperature)     header += ",temperature";
    if (flightParams.log.accel)           header += ",ax,ay,az";
    if (flightParams.log.gyro)            header += ",gx,gy,gz";
    if (flightParams.log.light)           header += ",light";
    if (flightParams.log.continuity)      header += ",continuity";
    if (flightParams.log.battery_voltage) header += ",battery_voltage";
    if (flightParams.log.events)          header += ",eventMarker";
    
    
    headerFile.println(header);
    headerFile.close();
    
    return true;
}

void startLogging() {
    logFile = LittleFS.open(currentLogFilename, "a");
    loggingActive = true;
    loggingStartTime = millis();
}

void stopLogging() {
    if (logFile) logFile.close();
    loggingActive = false;
}

void logData() {
    if (!loggingActive || !logFile) return;

    String line = String(millis() - loggingStartTime);

    if (flightParams.log.altitude)       line += "," + String(baro.data.altitude);
    if (flightParams.log.raw_altitude)   line += "," + String(baro.data.raw_altitude);
    if (flightParams.log.velocity)       line += "," + String(baro.data.velocity);
    if (flightParams.log.pressure)       line += "," + String(baro.data.pressure);
    if (flightParams.log.temperature)    line += "," + String(baro.data.temperature);
    //if (flightParams.log.accel)          line += "," + String(imu.data.ax) + "," + String(imu.data.ay) + "," + String(imu.data.az);
    //if (flightParams.log.gyro)           line += "," + String(imu.data.gx) + "," + String(imu.data.gy) + "," + String(imu.data.gz);
    if (flightParams.log.light)          line += "," + String(analogRead(LIGHT_SENSOR_PIN));
    if (flightParams.log.continuity)     line += "," + String(digitalRead(PYRO_CONTINUITY_PIN));
    if (flightParams.log.battery_voltage)line += "," + String(analogRead(BATTERY_VOLTAGE_PIN));
    if (flightParams.log.events)         line += "," + String(eventMarker);

    logFile.println(line);
}

void exportLogToSerial(int flightNum) {
    String filename = "/flight_" + String(flightNum) + ".csv";
    File file = LittleFS.open(filename, "r");
    if (!file) return;

    while (file.available()) {
        Serial.write(file.read());
    }
    file.close();
}

void startTestLog(uint16_t durationSec, bool printAfter, bool deleteAfter) {
    
    File testFile = LittleFS.open("/test_log.csv", "w");
    if (!testFile) {
        Serial.println("Failed to open test log file!");
        return;
    }
    unsigned long startTime = millis();

    
    while (millis() - startTime < durationSec * 1000UL) {
        
        baro.poll();
        //imu.poll();

        
        String line = String(millis());
        if (flightParams.log.altitude)        line += "," + String(baro.data.altitude);
        if (flightParams.log.raw_altitude)    line += "," + String(baro.data.raw_altitude);
        if (flightParams.log.velocity)        line += "," + String(baro.data.velocity);
        if (flightParams.log.pressure)        line += "," + String(baro.data.pressure);
        if (flightParams.log.temperature)     line += "," + String(baro.data.temperature);
        //if (flightParams.log.accel)           line += "," + String(imu.data.ax) + "," + String(imu.data.ay) + "," + String(imu.data.az);
        //if (flightParams.log.gyro)            line += "," + String(imu.data.gx) + "," + String(imu.data.gy) + "," + String(imu.data.gz);
        if (flightParams.log.light)           line += "," + String(analogRead(LIGHT_SENSOR_PIN));
        if (flightParams.log.continuity)      line += "," + String(digitalRead(PYRO_CONTINUITY_PIN));
        if (flightParams.log.battery_voltage) line += "," + String(analogRead(BATTERY_VOLTAGE_PIN));
        if (flightParams.log.events)          line += "," + String(eventMarker);

        
        testFile.println(line);
        delay(1000 / flightParams.POLL_FREQ);
    }
    testFile.close();

    
    if (printAfter) {
        File file = LittleFS.open("/test_log.csv", "r");
        if (file) {
            while (file.available()) {
                Serial.write(file.read());
            }
            file.close();
        } else {
            Serial.println("Unable to open test log for printing.");
        }
    }

    
    if (deleteAfter) {
        LittleFS.remove("/test_log.csv");
    }
}
