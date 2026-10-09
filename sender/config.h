#ifndef SENDER_CONFIG_H
#define SENDER_CONFIG_H

/*
 * Configuration: network parameters, pin definitions and hardware constants.
 */

#include <IPAddress.h>

// WiFi credentials and receiver (photo upload target)
const char* const ssid         = "火火大王专用";
const char* const password     = "yyt040801";
const char* const receiverIP   = "192.168.43.10";
const int         receiverPort = 8080;

// Backend WebSocket server
const char* const backendIP   = "192.168.43.20";
const int         backendPort = 5001;
const char* const backendPath = "/ws";

// Static IP
const IPAddress local_IP(192, 168, 43, 11);
const IPAddress gateway(192, 168, 43, 1);
const IPAddress subnet(255, 255, 255, 0);

// I2C pins (SDA 26 / SCL 27), overridable via build flags
#ifndef I2C_SDA
#define I2C_SDA 26
#endif
#ifndef I2C_SCL
#define I2C_SCL 27
#endif

// Number of video frame buffers
#define NUM_FRAME_BUFFERS 2

#endif // SENDER_CONFIG_H
