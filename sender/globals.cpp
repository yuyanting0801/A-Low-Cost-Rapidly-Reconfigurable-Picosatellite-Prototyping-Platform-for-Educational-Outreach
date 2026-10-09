#include "globals.h"

/*
 * Globals: definitions of all FreeRTOS objects, task handles and shared variables.
 */

TaskHandle_t commandTaskHandle   = NULL;
TaskHandle_t captureTaskHandle   = NULL;
TaskHandle_t sensorTaskHandle    = NULL;
TaskHandle_t transmitTaskHandle  = NULL;
TaskHandle_t captureFrameTaskHandle = NULL;
TaskHandle_t wsClientTaskHandle   = NULL;
TaskHandle_t wsSendTaskHandle     = NULL;

QueueHandle_t transmitQueue = NULL;
QueueHandle_t frameQueue = NULL;
QueueHandle_t wsSendQueue = NULL;

SemaphoreHandle_t i2cMutex   = NULL;
SemaphoreHandle_t wifiMutex  = NULL;
SemaphoreHandle_t frameMutex = NULL;
SemaphoreHandle_t wsMutex    = NULL;

SensorData latestSensorData;
bool sensorsInitialized = false;
FrameBuffer frameBuffers[NUM_FRAME_BUFFERS];
int currentWriteBuffer = 0;

WebSocketsClient webSocket;
bool wsConnected = false;

WebServer server(80);
