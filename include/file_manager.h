#pragma once

#include <Arduino.h>

bool initFileSystem();

String readFileToString(const char* path);
bool writeStringToFile(const char* path, const String& content);
bool fileExists(const char* path);
bool deleteFile(const char* path);
void listFiles();
