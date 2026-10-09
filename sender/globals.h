#ifndef SENDER_GLOBALS_H
#define SENDER_GLOBALS_H

/*
 * Globals: shared data structures and the global variables that carry them between tasks.
 */

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include <WebServer.h>
#include <WebSocketsClient.h>
#include "config.h"

/** Sensor readings */
struct SensorData {
  float temperature;
  float humidity;
  float uvi;
  float lux;
  float ax, ay, az;
  unsigned long timestamp;   // seconds since boot
};

/** Photo plus sensor data queued for upload */
struct TransmitPacket {
  uint8_t* imageData;
  size_t   imageSize;
  SensorData sensorData;
};

/** Reusable frame buffer (PSRAM) */
struct FrameBuffer {
  uint8_t* buffer;
  size_t length;
};

/** Outgoing WebSocket payload */
struct WsSendItem {
  uint8_t* data;
  size_t len;
};

extern TaskHandle_t commandTaskHandle;
extern TaskHandle_t captureTaskHandle;
extern TaskHandle_t sensorTaskHandle;
extern TaskHandle_t transmitTaskHandle;
extern TaskHandle_t captureFrameTaskHandle;
extern TaskHandle_t wsClientTaskHandle;
extern TaskHandle_t wsSendTaskHandle;

extern QueueHandle_t transmitQueue;
extern QueueHandle_t frameQueue;
extern QueueHandle_t wsSendQueue;

extern SemaphoreHandle_t i2cMutex;
extern SemaphoreHandle_t wifiMutex;
extern SemaphoreHandle_t frameMutex;
extern SemaphoreHandle_t wsMutex;

extern SensorData latestSensorData;
extern bool sensorsInitialized;
extern FrameBuffer frameBuffers[NUM_FRAME_BUFFERS];
extern int currentWriteBuffer;

extern WebSocketsClient webSocket;
extern bool wsConnected;

extern WebServer server;

#endif // SENDER_GLOBALS_H
