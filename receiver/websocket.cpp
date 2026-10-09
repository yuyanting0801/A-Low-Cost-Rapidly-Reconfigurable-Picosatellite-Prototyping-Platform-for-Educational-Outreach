#include "websocket.h"
#include "globals.h"
#include "config.h"
#include <ArduinoJson.h>
#include <SD.h>

/*
 * WebSocket implementation: connection handling, events and photo forwarding.
 */

// Forward declaration
static void webSocketEvent(WStype_t type, uint8_t* payload, size_t length);

void wsClientTask(void* pvParameters) {
  webSocket.begin(backendIP, backendPort, backendPath);
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(5000);
  webSocket.enableHeartbeat(15000, 5000, 3);

  while (1) {
    webSocket.loop();
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

static void webSocketEvent(WStype_t type, uint8_t* payload, size_t length) {
  switch (type) {
    case WStype_CONNECTED:
      Serial.println("WebSocket connected to backend");
      wsConnected = true;
      {
        StaticJsonDocument<128> idMsg;
        idMsg["type"] = "receiver";
        idMsg["ip"] = WiFi.localIP().toString();
        String idStr;
        serializeJson(idMsg, idStr);
        webSocket.sendTXT(idStr);
      }
      break;
    case WStype_DISCONNECTED:
      Serial.println("WebSocket disconnected");
      wsConnected = false;
      break;
    case WStype_TEXT:
      Serial.printf("WS command: %s\n", (char*)payload);
      break;
    default:
      break;
  }
}

void sendPhotoViaWS(const String& photoPath) {
  if (!wsConnected || !sdReady) return;

  File photo = SD.open(photoPath, FILE_READ);
  if (!photo) {
    Serial.println("Failed to open photo for WS send");
    return;
  }
  size_t fileSize = photo.size();
  if (fileSize == 0 || fileSize > 100 * 1024) {
    photo.close();
    Serial.println("Photo size invalid for WS");
    return;
  }

  uint8_t* buffer = (uint8_t*)malloc(fileSize);
  if (!buffer) {
    photo.close();
    return;
  }
  size_t readBytes = photo.read(buffer, fileSize);
  photo.close();

  if (readBytes == fileSize) {
    size_t totalLen = fileSize + 1;
    uint8_t* outBuf = (uint8_t*)malloc(totalLen);
    if (outBuf) {
      outBuf[0] = 0x02;
      memcpy(outBuf + 1, buffer, fileSize);
      webSocket.sendBIN(outBuf, totalLen);
      free(outBuf);
      Serial.printf("Photo sent via WS: %s (%d bytes)\n", photoPath.c_str(), fileSize);
    } else {
      Serial.println("Failed to allocate buffer for WS photo");
    }
  } else {
    Serial.println("Failed to read full photo");
  }
  free(buffer);
}
