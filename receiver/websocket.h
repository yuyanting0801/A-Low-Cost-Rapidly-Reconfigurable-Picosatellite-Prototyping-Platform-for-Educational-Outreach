#ifndef RECEIVER_WEBSOCKET_H
#define RECEIVER_WEBSOCKET_H

/*
 * WebSocket module: maintains the backend connection and forwards photos.
 */

#include <Arduino.h>

/** WebSocket client task: keeps the connection alive and runs the event loop */
void wsClientTask(void* pvParameters);

/** Send a photo to the backend over WebSocket (binary frame prefixed with 0x02) */
void sendPhotoViaWS(const String& photoPath);

#endif // RECEIVER_WEBSOCKET_H
