#pragma once

#include "reefie_config.h"

bool initLogFile();
void startLogging();
void stopLogging();
void logData();
void exportLogToSerial(int flightNum);
void startTestLog(uint16_t durationSec = 10, bool printAfter = true, bool deleteAfter = true);

