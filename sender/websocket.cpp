#include "websocket.h"
#include "config.h"
#include <ArduinoJson.h>

/*
 * WebSocket module: connection handling, event dispatch, command parsing and sending.
 */

static void webSocketEvent(WStype_t type, uint8_t* payload, size_t length);
static void processWebSocketCommand(const String& cmd);

/** Client task: sets up the connection and keeps the heartbeat running */
void wsClientTask(void* pvParameters) {
  webSocket.begin(backendIP, backendPort, backendPath);
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(2000);
  webSocket.enableHeartbeat(30000, 20000, 3);

  while (1) {
    webSocket.loop();
    vTaskDelay(pdMS_TO_TICKS(5));
  }
}

/** WebSocket event callback */
void webSocketEvent(WStype_t type, uint8_t* payload, size_t length) {
  switch (type) {
    case WStype_CONNECTED: {
      Serial.println("WebSocket connected");
      wsConnected = true;
      // Drop frames queued while disconnected
      WsSendItem* item;
      while (xQueueReceive(wsSendQueue, &item, 0) == pdTRUE) {
        if (item) {
          if (item->data) free(item->data);
          free(item);
        }
      }
      // Resume frame grabbing
      if (captureFrameTaskHandle) vTaskResume(captureFrameTaskHandle);
      // Identity handshake: announce this device to the backend as "sender"
      {
        StaticJsonDocument<128> idMsg;
        idMsg["type"] = "sender";
        idMsg["ip"] = WiFi.localIP().toString();
        String idStr;
        serializeJson(idMsg, idStr);
        if (xSemaphoreTake(wsMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
          webSocket.sendTXT(idStr);
          xSemaphoreGive(wsMutex);
        }
      }
      break;
    }
    case WStype_DISCONNECTED: {
      Serial.println("WebSocket disconnected");
      wsConnected = false;
      if (captureFrameTaskHandle) vTaskSuspend(captureFrameTaskHandle);
      WsSendItem* item;
      while (xQueueReceive(wsSendQueue, &item, 0) == pdTRUE) {
        if (item) {
          if (item->data) free(item->data);
          free(item);
        }
      }
      break;
    }
    case WStype_TEXT:
      processWebSocketCommand(String((char*)payload));
      break;
    default:
      break;
  }
}

/** Handle a text command from the backend */
static void processWebSocketCommand(const String& cmd) {
  StaticJsonDocument<256> doc;
  if (deserializeJson(doc, cmd)) return;
  String type = doc["type"];
  if (type == "capture") {
    if (captureTaskHandle) xTaskNotifyGive(captureTaskHandle);
  } else if (type == "start_stream") {
    if (captureFrameTaskHandle) vTaskResume(captureFrameTaskHandle);
  } else if (type == "stop_stream") {
    if (captureFrameTaskHandle) vTaskSuspend(captureFrameTaskHandle);
  }
}

/** Send sensor data to the backend as JSON */
void sendSensorDataViaWS(const SensorData& data) {
  if (!wsConnected) return;
  StaticJsonDocument<512> doc;
  doc["type"] = "sensor";
  doc["temperature"] = data.temperature;
  doc["humidity"]    = data.humidity;
  doc["uvi"]         = data.uvi;
  doc["lux"]         = data.lux;
  doc["ax"]          = data.ax;
  doc["ay"]          = data.ay;
  doc["az"]          = data.az;
  doc["timestamp"]   = data.timestamp;
  String out;
  serializeJson(doc, out);
  if (xSemaphoreTake(wsMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
    webSocket.sendTXT(out);
    xSemaphoreGive(wsMutex);
  }
}

/** Send a video frame as binary, with 0x01 as the first (type) byte */
void sendFrameViaWS(const uint8_t* frame, size_t length) {
  if (!wsConnected) return;
  if (uxQueueMessagesWaiting(wsSendQueue) > 5) return;
  WsSendItem* item = (WsSendItem*)malloc(sizeof(WsSendItem));
  if (!item) return;
  item->data = (uint8_t*)malloc(length + 1);
  if (!item->data) {
    free(item);
    return;
  }
  item->data[0] = 0x01;
  memcpy(item->data + 1, frame, length);
  item->len = length + 1;
  if (xQueueSend(wsSendQueue, &item, 0) != pdTRUE) {
    free(item->data);
    free(item);
  }
}

/** Send task: drains the send queue to the WebSocket */
void wsSendTask(void* pvParameters) {
  while (1) {
    WsSendItem* item = NULL;
    if (xQueueReceive(wsSendQueue, &item, portMAX_DELAY) == pdTRUE) {
      if (wsConnected && item) {
        if (xSemaphoreTake(wsMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
          webSocket.sendBIN(item->data, item->len);
          xSemaphoreGive(wsMutex);
        }
      }
      if (item) {
        if (item->data) free(item->data);
        free(item);
      }
    }
  }
}
