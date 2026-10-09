#include "web_server.h"
#include "globals.h"
#include <ArduinoJson.h>

/*
 * HTTP server: POST /capture, GET /sensor and GET /stream (MJPEG).
 */

/** POST /capture: trigger a single photo capture */
void handleCapture() {
  if (captureTaskHandle) {
    xTaskNotifyGive(captureTaskHandle);
    server.send(200, "text/plain", "Capture triggered");
  } else {
    server.send(500, "text/plain", "Capture task not ready");
  }
}

/** GET /sensor: latest sensor data as JSON */
void handleSensor() {
  SensorData data;
  if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
    data = latestSensorData;
    xSemaphoreGive(i2cMutex);
  } else {
    data.temperature = -999;
    data.humidity = -999;
    data.uvi = -999;
    data.lux = -999;
    data.ax = data.ay = data.az = 0;
    data.timestamp = millis() / 1000;
  }
  StaticJsonDocument<256> doc;
  doc["temperature"] = data.temperature;
  doc["humidity"]    = data.humidity;
  doc["uvi"]         = data.uvi;
  doc["lux"]         = data.lux;
  doc["ax"]          = data.ax;
  doc["ay"]          = data.ay;
  doc["az"]          = data.az;
  doc["timestamp"]   = data.timestamp;
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

/** GET /stream: MJPEG stream with the latest sensor data in each part header */
void handleStream() {
  server.sendHeader("Content-Type", "multipart/x-mixed-replace; boundary=--boundary");
  server.sendHeader("Cache-Control", "no-cache");
  server.sendHeader("Connection", "close");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "multipart/x-mixed-replace; boundary=--boundary", "");

  while (server.client().connected()) {
    FrameBuffer* fbBuf = NULL;
    if (xQueueReceive(frameQueue, &fbBuf, pdMS_TO_TICKS(50)) != pdTRUE) {
      server.sendContent("--boundary\r\n\r\n");
      continue;
    }
    SensorData data;
    if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(20)) == pdTRUE) {
      data = latestSensorData;
      xSemaphoreGive(i2cMutex);
    } else {
      data = latestSensorData;
    }
    StaticJsonDocument<256> doc;
    doc["temperature"] = data.temperature;
    doc["humidity"]    = data.humidity;
    doc["uvi"]         = data.uvi;
    doc["lux"]         = data.lux;
    doc["ax"]          = data.ax;
    doc["ay"]          = data.ay;
    doc["az"]          = data.az;
    doc["timestamp"]   = data.timestamp;
    String sensorJSON;
    serializeJson(doc, sensorJSON);
    String header = "--boundary\r\nContent-Type: image/jpeg\r\nContent-Length: " + String(fbBuf->length) + "\r\nX-Sensor-Data: " + sensorJSON + "\r\n\r\n";
    server.sendContent(header);
    server.sendContent((const char*)fbBuf->buffer, fbBuf->length);
    server.sendContent("\r\n");
  }
  Serial.println("Stream client disconnected");
}

/** HTTP task: registers the routes and serves requests in a loop */
void commandTask(void* pvParameters) {
  server.on("/capture", HTTP_POST, handleCapture);
  server.on("/sensor",  HTTP_GET,  handleSensor);
  server.on("/stream",  HTTP_GET,  handleStream);
  server.begin();
  Serial.println("Web server started");
  while (1) {
    server.handleClient();
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}
