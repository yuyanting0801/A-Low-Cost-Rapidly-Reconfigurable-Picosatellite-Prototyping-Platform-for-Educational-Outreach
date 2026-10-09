#ifndef RECEIVER_CONFIG_H
#define RECEIVER_CONFIG_H

/*
 * Configuration: network parameters, pin assignments and ports for the Receiver.
 */

#include <IPAddress.h>

const char* const ssid     = "火火大王专用";
const char* const password = "yyt040801";

const IPAddress local_IP(192, 168, 1, 10);
const IPAddress gateway(192, 168, 1, 1);
const IPAddress subnet(255, 255, 255, 0);

// Backend WebSocket server
const char* const backendIP   = "192.168.1.103";
const int         backendPort = 5001;
const char* const backendPath = "/ws";

// SD card SPI pins
#define SD_CS     5
#define SPI_MOSI  13
#define SPI_MISO  12
#define SPI_SCK   14

// Server ports
const int TCP_PORT   = 8080;   // upload port (POST /upload)
const int HTTP_PORT  = 80;     // file server port

// Cap on queued WebSocket frames
#define MAX_WS_QUEUE_DEPTH 3

#endif // RECEIVER_CONFIG_H
