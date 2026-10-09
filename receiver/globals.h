#ifndef RECEIVER_GLOBALS_H
#define RECEIVER_GLOBALS_H

/*
 * Globals: shared variables used across the Receiver modules.
 */

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <WebSocketsClient.h>

extern bool sdReady;
extern String latestPhotoPath;
extern WebSocketsClient webSocket;
extern bool wsConnected;

extern TaskHandle_t tcpServerTaskHandle;
extern TaskHandle_t httpServerTaskHandle;
extern TaskHandle_t wsClientTaskHandle;

#endif // RECEIVER_GLOBALS_H
