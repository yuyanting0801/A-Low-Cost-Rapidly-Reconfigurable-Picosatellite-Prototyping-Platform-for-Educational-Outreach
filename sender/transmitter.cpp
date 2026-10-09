#include "transmitter.h"
#include "globals.h"
#include "config.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

/*
 * Transmitter: POSTs the photo to http://<receiver>:8080/upload with the
 * X-File-Name and X-Sensor-Data headers, plus the raw JPEG as the body.
 */

void transmitTask(void* pvParameters) {
  static unsigned long photoCounter = 0;
  while (1) {
    TransmitPacket* pkt = NULL;
    if (xQueueReceive(transmitQueue, &pkt, portMAX_DELAY) == pdTRUE) {
      if (WiFi.status() != WL_CONNECTED) {
        free(pkt->imageData);
        free(pkt);
        continue;
      }

      // Sensor data goes into the X-Sensor-Data header as JSON
      StaticJsonDocument<512> doc;
      doc["temperature"] = pkt->sensorData.temperature;
      doc["humidity"]    = pkt->sensorData.humidity;
      doc["uvi"]         = pkt->sensorData.uvi;
      doc["lux"]         = pkt->sensorData.lux;
      doc["ax"]          = pkt->sensorData.ax;
      doc["ay"]          = pkt->sensorData.ay;
      doc["az"]          = pkt->sensorData.az;
      doc["timestamp"]   = pkt->sensorData.timestamp;
      String sensorJSON;
      serializeJson(doc, sensorJSON);

      photoCounter++;
      String filename = String("photo_") + String(photoCounter) + String(".jpg");

      if (xSemaphoreTake(wifiMutex, pdMS_TO_TICKS(5000)) == pdTRUE) {
        HTTPClient http;
        http.begin(String("http://") + receiverIP + ":" + receiverPort + "/upload");
        http.addHeader("Content-Type", "application/octet-stream");
        http.addHeader("X-File-Name", filename);
        http.addHeader("X-Sensor-Data", sensorJSON);
        http.addHeader("Content-Length", String(pkt->imageSize));

        int httpCode = http.POST(pkt->imageData, pkt->imageSize);
        if (httpCode > 0 && httpCode == 200) {
          Serial.printf("Upload success: %s (%d bytes)\n", filename.c_str(), pkt->imageSize);
        } else {
          Serial.printf("Upload failed, HTTP code: %d\n", httpCode);
        }
        http.end();
        xSemaphoreGive(wifiMutex);
      }

      free(pkt->imageData);
      free(pkt);
    }
  }
}
