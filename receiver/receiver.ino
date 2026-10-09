/*
 * Receiver entry point.
 * Responsibilities: system init, module wiring, FreeRTOS task creation.
 * Logic lives in these modules:
 *   - config.h:           network / SD card / port configuration
 *   - globals.h/cpp:      shared global definitions
 *   - sd_card.h/cpp:      SD card init and directory setup
 *   - wifi_manager.h/cpp: WiFi connection
 *   - tcp_server.h/cpp:   TCP upload server
 *   - http_server.h/cpp:  HTTP file server
 *   - websocket.h/cpp:    WebSocket client
 */

#include <Arduino.h>
#include "config.h"
#include "globals.h"
#include "sd_card.h"
#include "wifi_manager.h"
#include "tcp_server.h"
#include "http_server.h"
#include "websocket.h"

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n========================================");
  Serial.println("ESP32 Receiver - Modularized Version");
  Serial.println("========================================");

  initSDCard();
  connectToWiFi();

  // Start FreeRTOS tasks, all pinned to core 0
  xTaskCreatePinnedToCore(tcpServerTask,   "TCPServer",   8192, NULL, 1, &tcpServerTaskHandle,   0);
  xTaskCreatePinnedToCore(httpServerTask,  "HTTPServer",  8192, NULL, 1, &httpServerTaskHandle,  0);
  xTaskCreatePinnedToCore(wsClientTask,    "WSClientTask", 8192, NULL, 2, &wsClientTaskHandle,   0);

  Serial.println("System initialized");
  Serial.printf("   TCP server (upload)  : %s:%d\n", WiFi.localIP().toString().c_str(), TCP_PORT);
  Serial.printf("   HTTP server (files)  : %s:%d\n", WiFi.localIP().toString().c_str(), HTTP_PORT);
  Serial.printf("   Backend WS: ws://%s:%d%s\n", backendIP, backendPort, backendPath);
  Serial.printf("   WS queue discard threshold: >%d\n", MAX_WS_QUEUE_DEPTH);
}

void loop() {
  vTaskDelete(NULL);
}
