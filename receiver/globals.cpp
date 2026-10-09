#include "globals.h"

/*
 * Global variable definitions.
 */

bool sdReady = false;
String latestPhotoPath = "";
WebSocketsClient webSocket;
bool wsConnected = false;

TaskHandle_t tcpServerTaskHandle   = NULL;
TaskHandle_t httpServerTaskHandle  = NULL;
TaskHandle_t wsClientTaskHandle    = NULL;
