#ifdef EXPORT_TOOL

#include <Arduino.h>
#include <LittleFS.h>
#include "file_manager.h"
#include "reefie_config.h"

#define MAX_FILES 20

// Global array to hold file names.
String fileList[MAX_FILES];
int fileCount = 0;

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
  // Signal start of export transmission.
  Serial.print("EXPORT_BEGIN ");
  Serial.println(fname);
  File f = LittleFS.open("/" + fname, "r");
  if (!f) {
    Serial.println("Could not open file.");
    return;
  }
  // Send file content.
  while (f.available()) {
    Serial.write(f.read());
  }
  f.close();
  Serial.println(); // End-of-line after file content.
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

void processCommand(String cmd) {
  cmd.trim();
  cmd.toUpperCase(); // Make command matching case-insensitive.
  if (cmd.startsWith("LIST")) {
    listFilesSerial();
  } else if (cmd.startsWith("EXPORT")) {
    // Expect: EXPORT <number>
    int spaceIdx = cmd.indexOf(' ');
    if (spaceIdx == -1) {
      Serial.println("Usage: EXPORT <file number>");
    } else {
      String numStr = cmd.substring(spaceIdx + 1);
      int index = numStr.toInt();
      exportFile(index);
    }
  } else if (cmd.startsWith("DELETE")) {
    // Expect: DELETE <number>
    int spaceIdx = cmd.indexOf(' ');
    if (spaceIdx == -1) {
      Serial.println("Usage: DELETE <file number>");
    } else {
      String numStr = cmd.substring(spaceIdx + 1);
      int index = numStr.toInt();
      deleteFileByIndex(index);
    }
  } else {
    Serial.println("Unknown command.");
    Serial.println("Available: LIST, EXPORT <num>, DELETE <num>");
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);
  if (!LittleFS.begin()) {
    Serial.println("LittleFS initialization failed!");
    while (1) delay(1000);
  }
  Serial.println("Reefie File Manager Interface Ready.");
  Serial.println("Type LIST to list files.");
}

void loop() {
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    processCommand(input);
  }
}


#endif