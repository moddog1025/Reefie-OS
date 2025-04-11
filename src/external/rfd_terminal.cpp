#include <Arduino.h>
#include <LittleFS.h>
#include "file_manager.h"
#include "reefie_config.h"
#include "rfd_terminal.h"

// Global array to hold file names.
String fileList[MAX_FILES];
int fileCount = 0;

// For flight parameters update.
bool inSetParamsMode = false;
String paramsBuffer = "";

void listFilesSerial() {
  fileCount = 0;
  File root = LittleFS.open("/");
  File file = root.openNextFile();
  while (file && fileCount < MAX_FILES) {
    String fname = file.name();
    fileList[fileCount] = fname;
    Serial.print(fileCount);
    Serial.print(": ");
    Serial.println(fname);
    fileCount++;
    file = root.openNextFile();
  }
  root.close();
  Serial.println("Enter file number:");
}

void exportFile(int index) {
  if (index < 0 || index >= fileCount) {
    Serial.println("Invalid file index.");
    return;
  }
  String fname = fileList[index];
  Serial.print("EXPORT_BEGIN ");
  Serial.println(fname);
  File f = LittleFS.open("/" + fname, "r");
  if (!f) {
    Serial.println("Could not open file.");
    return;
  }
  while (f.available()) {
    Serial.write(f.read());
  }
  f.close();
  Serial.println();
  Serial.println("EXPORT_END");
}

void deleteFileByIndex(int index) {
  if (index < 0 || index >= fileCount) {
    Serial.println("Invalid file index.");
    return;
  }
  String fname = "/" + fileList[index];
  bool success = LittleFS.remove(fname);
  if (success) {
    Serial.print("Deleted file: ");
    Serial.println(fname);
  } else {
    Serial.print("Failed to delete file: ");
    Serial.println(fname);
  }
}

void getFlightParams() {
  // Read and transmit flight_params.json wrapped with markers.
  File f = LittleFS.open("/flight_params.json", "r");
  if (!f) {
    Serial.println("Could not open flight_params.json.");
    return;
  }
  Serial.println("PARAMS_BEGIN");
  while (f.available()) {
    Serial.write(f.read());
  }
  f.close();
  Serial.println();
  Serial.println("PARAMS_END");
}

// This function handles normal commands when not in SETPARAMS mode.
void processCommand(String cmd) {
  cmd.trim();
  cmd.toUpperCase();
  
  if (cmd.startsWith("LIST")) {
    listFilesSerial();
  } else if (cmd.startsWith("EXPORT")) {
    int spaceIdx = cmd.indexOf(' ');
    if (spaceIdx == -1) {
      Serial.println("Usage: EXPORT <file number>");
    } else {
      String numStr = cmd.substring(spaceIdx + 1);
      int index = numStr.toInt();
      exportFile(index);
    }
  } else if (cmd.startsWith("DELETE")) {
    int spaceIdx = cmd.indexOf(' ');
    if (spaceIdx == -1) {
      Serial.println("Usage: DELETE <file number>");
    } else {
      String numStr = cmd.substring(spaceIdx + 1);
      int index = numStr.toInt();
      deleteFileByIndex(index);
    }
  } else if (cmd.startsWith("GETPARAMS")) {
    getFlightParams();
  } else if (cmd.startsWith("SETPARAMS_BEGIN")) {
    // Begin flight parameters update.
    inSetParamsMode = true;
    paramsBuffer = "";
    Serial.println("Ready to receive flight parameters.");
  } else {
    Serial.println("Unknown command.");
    Serial.println("Available: LIST, EXPORT <num>, DELETE <num>, GETPARAMS, SETPARAMS_BEGIN ... SETPARAMS_END");
  }
}

void loopCommand(String line) {
  // This function is called for every new line read from Serial.
  if (inSetParamsMode) {
    // Check if this line indicates the end of SETPARAMS.
    if (line.indexOf("SETPARAMS_END") != -1) {
      inSetParamsMode = false;
      // Write the received data into flight_params.json.
      File f = LittleFS.open("/flight_params.json", "w");
      if (!f) {
        Serial.println("Failed to open flight_params.json for writing.");
      } else {
        f.print(paramsBuffer);
        f.close();
        Serial.println("Flight parameters updated successfully.");
      }
    } else {
      // Accumulate the line.
      paramsBuffer += line + "\n";
    }
  } else {
    processCommand(line);
  }
}
