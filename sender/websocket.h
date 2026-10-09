#ifndef SENDER_WEBSOCKET_H
#define SENDER_WEBSOCKET_H

/*
 * WebSocket module: maintains the connection to the backend and exchanges messages.
 */

#include <Arduino.h>
#include "globals.h"

/** Client task: runs the connection and event loop */
void wsClientTask(void* pvParameters);

/** Send task: takes payloads from the send queue and writes them to the socket */
void wsSendTask(void* pvParameters);

/** Send sensor data to the backend as JSON (type "sensor") */
void sendSensorDataViaWS(const SensorData& data);

/** Send a video frame as a binary message prefixed with the 0x01 type byte */
void sendFrameViaWS(const uint8_t* frame, size_t length);

#endif // SENDER_WEBSOCKET_H
