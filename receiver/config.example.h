#ifndef RECEIVER_CONFIG_H
#define RECEIVER_CONFIG_H

/*
 * Configuration: network parameters, pin assignments and ports for the Receiver.
 * Copy this file to config.h and fill in your own values.
 *
 * IP requirements:
 * - Same LAN and subnet as the sender board and the backend PC.
 * - Static IP outside the router's DHCP pool.
 */

#include <IPAddress.h>

const char* const ssid     = "YOUR_WIFI_SSID";
const char* const password = "YOUR_WIFI_PASSWORD";

// Static IP of this board (must match receiverIP in the sender's config.h)
const IPAddress local_IP(0, 0, 0, 0);
const IPAddress gateway(0, 0, 0, 0);
const IPAddress subnet(255, 255, 255, 0);   // adjust to your network

// Backend WebSocket server
const char* const backendIP   = "BACKEND_IP";     // IP of the PC running code/server
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
