#include "sd_card.h"
#include "globals.h"
#include "config.h"
#include <SD.h>
#include <SPI.h>

/*
 * SD card module implementation.
 */

void initSDCard() {
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI, SD_CS);
  delay(100);
  if (!SD.begin(SD_CS)) {
    Serial.println("SD Card init failed!");
    sdReady = false;
    return;
  }
  sdReady = true;
  Serial.println("SD Card ready");

  const char* dirs[] = {"/images", "/sensor_data"};
  for (const char* d : dirs) {
    if (!SD.exists(d)) {
      if (SD.mkdir(d)) Serial.printf("   Created dir: %s\n", d);
    }
  }
}

String getPhotoListJson() {
  File root = SD.open("/images");
  if (!root) return "[]";
  String json = "[";
  bool first = true;
  File file;
  while (file = root.openNextFile()) {
    if (!file.isDirectory()) {
      if (!first) json += ",";
      json += "{\"name\":\"" + String(file.name()) + "\",\"size\":" + String(file.size()) + "}";
      first = false;
    }
    file.close();
  }
  root.close();
  json += "]";
  return json;
}

String getSensorDataListJson() {
  File root = SD.open("/sensor_data");
  if (!root) return "[]";
  String json = "[";
  bool first = true;
  File file;
  while (file = root.openNextFile()) {
    if (!file.isDirectory() && String(file.name()).endsWith(".json")) {
      if (!first) json += ",";
      json += "{\"name\":\"" + String(file.name()) + "\",\"size\":" + String(file.size()) + "}";
      first = false;
    }
    file.close();
  }
  root.close();
  json += "]";
  return json;
}
