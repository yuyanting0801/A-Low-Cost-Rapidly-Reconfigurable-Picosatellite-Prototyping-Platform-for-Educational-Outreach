#ifndef RECEIVER_TCP_SERVER_H
#define RECEIVER_TCP_SERVER_H

/*
 * TCP upload server: receives photos and sensor data POSTed by the Sender.
 */

#include <Arduino.h>
#include <WiFi.h>

/** TCP server task: listen on the upload port and store incoming files */
void tcpServerTask(void* pvParameters);

#endif // RECEIVER_TCP_SERVER_H
