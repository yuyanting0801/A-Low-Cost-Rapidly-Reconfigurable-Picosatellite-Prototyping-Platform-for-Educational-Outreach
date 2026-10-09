/*
 * Sender entry point: system init, module startup and FreeRTOS task creation.
 * Logic is split across these modules:
 *   - config.h:           WiFi/network/pin configuration
 *   - globals.h/cpp:      data structures and shared globals
 *   - camera.h/cpp:       camera init, frame capture, photo capture
 *   - sensors.h/cpp:      sensor readout and periodic reporting
 *   - wifi_manager.h/cpp: WiFi connection management
 *   - websocket.h/cpp:    WebSocket client communication
 *   - web_server.h/cpp:   HTTP server endpoints
 *   - transmitter.h/cpp:  photo upload to the receiver
 */

#include <Arduino.h>
#include <Wire.h>
#include "config.h"
#include "globals.h"
#include "camera.h"
#include "sensors.h"
#include "wifi_manager.h"
#include "websocket.h"
#include "web_server.h"
#include "transmitter.h"

void createFreeRTOSObjects();
void createAndStartTasks();

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n========================================");
  Serial.println("ESP32 Sender - Modularized Version");
  Serial.println("========================================");

  if (!psramFound()) {
    Serial.println("No PSRAM! System requires PSRAM.");
    while (1) delay(1000);
  }
  Serial.printf("PSRAM size: %d bytes\n", ESP.getPsramSize());

  Wire.setPins(I2C_SDA, I2C_SCL);
  Wire.begin();
  Wire.setClock(100000);
  delay(100);

  sensorsInitialized = initSensors();

  if (!initCamera()) {
    Serial.println("Camera init failed! Restarting...");
    ESP.restart();
  }

  connectToWiFi();

  createFreeRTOSObjects();

  // Pre-allocate the PSRAM frame buffers (25 KB each, matching the camera frame limit)
  for (int i = 0; i < NUM_FRAME_BUFFERS; i++) {
    frameBuffers[i].buffer = (uint8_t*)ps_malloc(25 * 1024);
    if (!frameBuffers[i].buffer) {
      Serial.println("Failed to allocate PSRAM for frame buffer");
      while (1) delay(100);
    }
    frameBuffers[i].length = 0;
  }

  createAndStartTasks();

  Serial.println("System initialized");
  Serial.printf("   Sender IP: %s\n", WiFi.localIP().toString().c_str());
  Serial.printf("   Backend WS: ws://%s:%d%s\n", backendIP, backendPort, backendPath);
}

void loop() {
  vTaskDelete(NULL);
}

void createFreeRTOSObjects() {
  transmitQueue = xQueueCreate(5, sizeof(TransmitPacket*));
  frameQueue = xQueueCreate(5, sizeof(FrameBuffer*));
  wsSendQueue = xQueueCreate(10, sizeof(WsSendItem*));
  i2cMutex  = xSemaphoreCreateMutex();
  wifiMutex = xSemaphoreCreateMutex();
  frameMutex = xSemaphoreCreateMutex();
  wsMutex   = xSemaphoreCreateMutex();

  if (!transmitQueue || !frameQueue || !wsSendQueue ||
      !i2cMutex || !wifiMutex || !frameMutex || !wsMutex) {
    Serial.println("Failed to create FreeRTOS objects!");
    ESP.restart();
  }
}

// Task creation and core assignment
void createAndStartTasks() {
  // Core 0: network stack, HTTP, WebSocket I/O and upload tasks
  xTaskCreatePinnedToCore(commandTask,        "CommandTask",   8192, NULL, 1, &commandTaskHandle,   0);
  xTaskCreatePinnedToCore(wsClientTask,       "WSClientTask",  8192, NULL, 4, &wsClientTaskHandle,  0);
  xTaskCreatePinnedToCore(wsSendTask,         "WSSendTask",    4096, NULL, 2, &wsSendTaskHandle,    0);
  xTaskCreatePinnedToCore(transmitTask,       "TransmitTask",  8192, NULL, 2, &transmitTaskHandle,  0);

  // Core 1: acquisition-heavy tasks (frame grab, sensor read, photo capture)
  xTaskCreatePinnedToCore(captureFrameTask,   "CaptureFrame", 8192, NULL, 3, &captureFrameTaskHandle, 1);
  xTaskCreatePinnedToCore(sensorTask,         "SensorTask",    4096, NULL, 2, &sensorTaskHandle,    1);
  xTaskCreatePinnedToCore(captureTask,        "CaptureTask",   8192, NULL, 1, &captureTaskHandle,   1);

  // Frame grabbing stays suspended until the WebSocket connects
  vTaskSuspend(captureFrameTaskHandle);
}
