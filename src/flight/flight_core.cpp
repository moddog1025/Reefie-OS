#ifndef REEFIE_FLIGHT_DESK_TERMINAL

#include "reefie_config.h"
#include "telemetry.h"
#include "board_init.h"
//#include <WiFi.h>
//#include <AsyncTCP.h>
//#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

//const char* ssid = "Reefie_v3L";
//const char* password = "BPE12345";

//AsyncWebServer server(80);

enum FlightState {
    PAD_IDLE,
    ASCENT,
    REEFED_DESCENT,
    MAIN_DESCENT,
    TOUCHDOWN
};

FlightState currentState = PAD_IDLE;


const unsigned long LAUNCH_DEBOUNCE_TIME = 200;
const unsigned long APOGEE_DEBOUNCE_TIME = 4000;
const unsigned long DISREEF_DEBOUNCE_TIME = 500;
const unsigned long TOUCHDOWN_DEBOUNCE_TIME = 1000;

bool inFlight = false;

bool launchedCheck();
bool chuteDeployedCheck();
bool disreefAltitudeCheck();
bool touchdownCheck();
bool checkStateChange(bool condition, unsigned long& lastTrueTime, unsigned long debounceTime);

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





/*
void initWebDash() {
  
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  
  
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
      File file = LittleFS.open("/index.html", "r");
      if (!file) {
         request->send(404, "text/plain", "File not found");
         return;
      }
      String html = file.readString();
      file.close();
      request->send(200, "text/html", html);
  });
  
  
  server.on("/data", HTTP_GET, [](AsyncWebServerRequest *request) {
      String displayState = "";
      if (currentState == PAD_IDLE) displayState = "PAD_IDLE";
      else if (currentState == ASCENT) displayState = "ASCENT";
      else if (currentState == REEFED_DESCENT) displayState = "REEFED_DESCENT";
      else if (currentState == MAIN_DESCENT) displayState = "MAIN_DESCENT";
      else if (currentState == TOUCHDOWN) displayState = "TOUCHDOWN";

      StaticJsonDocument<256> doc;
      doc["state"] = displayState;
      doc["altitude"] = Telem.altitude;
      doc["velocity"] = Telem.velocity;
      doc["accelMag"] = Telem.accel_mag;
      doc["light"] = Telem.light;

      String json;
      serializeJson(doc, json);
      request->send(200, "application/json", json);
  });
  
  server.begin();
  Serial.println("HTTP server started");
}

*/





void setup() {
    Serial.begin(115200);
    delay(1000);
    
    initializeBoard();
    initLogFile();
    //initWebDash();
}



void loop() {

    static unsigned long launchTimer = 0;
    static unsigned long apogeeTimer = 0;
    static unsigned long disreefTimer = 0;
    static unsigned long touchdownTimer = 0;
    static unsigned long lastPollTime = 0;

    unsigned long interval = 1000 / flightParams.POLL_FREQ;
    
    if (millis() - lastPollTime >= interval) {
      updateTelemetry();
      if (inFlight) {
          logData();
      }
      lastPollTime = millis();
    }
    
    switch (currentState) {
      case PAD_IDLE:
        if (checkStateChange(launchedCheck(), launchTimer, LAUNCH_DEBOUNCE_TIME)) {
            inFlight = true;
            currentState = ASCENT;
            startLogging();
        }
        break;
      
      case ASCENT:
        if (checkStateChange(chuteDeployedCheck(), apogeeTimer, APOGEE_DEBOUNCE_TIME))  {
            currentState = REEFED_DESCENT;
        }
        break;
      
      case REEFED_DESCENT:
        if (checkStateChange(disreefAltitudeCheck(), disreefTimer, DISREEF_DEBOUNCE_TIME))  {
            digitalWrite(PYRO_FIRE_PIN, HIGH);
            currentState = MAIN_DESCENT;
        }
        break;
      
      case MAIN_DESCENT:
        if (checkStateChange(touchdownCheck(), touchdownTimer, TOUCHDOWN_DEBOUNCE_TIME)) {
            currentState = TOUCHDOWN;
        }
        break;
      
      case TOUCHDOWN:
        stopLogging();
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
  return abs(Telem.velocity) <= flightParams.VELOC_THRESHOLD;
}

#endif