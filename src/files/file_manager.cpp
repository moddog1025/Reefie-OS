#include "file_manager.h"
#include "reefie_config.h"

bool initFileSystem() {
    return LittleFS.begin();
}

String readFileToString(const char* path) {
    if (!LittleFS.exists(path)) return "";

    File file = LittleFS.open(path, "r");
    if (!file) return "";

    String contents = file.readString();
    file.close();
    return contents;
}

bool writeStringToFile(const char* path, const String& content) {
    File file = LittleFS.open(path, "w");
    if (!file) return false;

    file.print(content);
    file.close();
    return true;
}

bool fileExists(const char* path) {
    return LittleFS.exists(path);
}

bool deleteFile(const char* path) {
    return LittleFS.remove(path);
}

void listFiles() {
    File root = LittleFS.open("/");
    File file = root.openNextFile();
    while (file) {
        Serial.println(file.name());
        file = root.openNextFile();
    }
}
