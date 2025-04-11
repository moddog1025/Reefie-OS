#pragma once

#include <Arduino.h>
#include "reefie_config.h"

#define MAX_FILES 20

// Global variables used for file management.
extern String fileList[MAX_FILES];
extern int fileCount;

// Globals for flight parameters update.
extern bool inSetParamsMode;
extern String paramsBuffer;

// Function declarations.
void listFilesSerial();
void exportFile(int index);
void deleteFileByIndex(int index);
void getFlightParams();
void processCommand(String cmd);
void loopCommand(String line);
