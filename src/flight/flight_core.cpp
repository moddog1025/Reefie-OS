#ifndef EXPORT_TOOL

#include "reefie_config.h"
#include "telemetry.h"
#include "board_init.h"

enum FlightState {
    PAD_IDLE,
    ASCENT,
    REEFED_DESCENT,
    MAIN_DESCENT,
    TOUCHDOWN
};

FlightState currentState = PAD_IDLE;

const unsigned long LAUNCH_DEBOUNCE_TIME = 250;
const unsigned long APOGEE_DEBOUNCE_TIME = 2000;
const unsigned long DISREEF_DEBOUNCE_TIME = 1000;
const unsigned long TOUCHDOWN_DEBOUNCE_TIME = 1000;

bool inFlight = false;

bool launchedCheck();
bool chuteDeployedCheck();
bool disreefAltitudeCheck();
bool touchdownCheck();

bool checkStateChange(bool condition, unsigned long& lastTrueTime, unsigned long debounceTime) {
    if (condition) {
      if (lastTrueTime == 0) {
        lastTrueTime = millis();
      } else if (millis() - lastTrueTime >= debounceTime) {
        lastTrueTime = 0;
        return true;
      }
    } else {
      lastTrueTime = 0;
    }
    return false;
}
  

void setup() {
    Serial.begin(115200);
    delay(1000);

    initializeBoard();
    initLogFile();
}


void loop() {

    static unsigned long launchTimer = 0;
    static unsigned long apogeeTimer = 0;
    static unsigned long disreefTimer = 0;
    static unsigned long touchdownTimer = 0;

    updateTelemetry();

    if (inFlight) {
        static unsigned long lastLogTime = 0;
        unsigned long interval = 1000 / flightParams.POLL_FREQ; 

        if (millis() - lastLogTime >= interval) {
            logData();
            lastLogTime = millis();
        }
    }

    switch (currentState) {
    
        case PAD_IDLE:
          if (checkStateChange(launchedCheck(), launchTimer, LAUNCH_DEBOUNCE_TIME))
          {
            inFlight = true;
            currentState = ASCENT;
            startLogging();
          }
          break;
    
        case ASCENT:
          if (checkStateChange(chuteDeployedCheck(), apogeeTimer, APOGEE_DEBOUNCE_TIME)) 
          {
            currentState = REEFED_DESCENT;
          }
          break;
    
        case REEFED_DESCENT:
          if (checkStateChange(disreefAltitudeCheck(), disreefTimer, DISREEF_DEBOUNCE_TIME)) 
          {
            digitalWrite(PYRO_FIRE_PIN, HIGH);
            currentState = MAIN_DESCENT;
          }
          break;
    
        case MAIN_DESCENT:
          if (checkStateChange(touchdownCheck(), touchdownTimer, TOUCHDOWN_DEBOUNCE_TIME))
          {
            currentState = TOUCHDOWN;
          }
          break;
    
        case TOUCHDOWN:
          stopLogging();
          while(1);
          break;
      }
}


bool launchedCheck() {
    return Telem.accel_mag >= flightParams.ACCEL_THRESHOLD;
}
  
bool chuteDeployedCheck() {
    return Telem.light >= flightParams.LIGHT_THRESHOLD;
}

bool disreefAltitudeCheck() {
    return baro.data.altitude <= flightParams.DISREEF_ALT;
}

bool touchdownCheck() {
    return abs(Telem.velocity) < 0.5;
}

#endif
