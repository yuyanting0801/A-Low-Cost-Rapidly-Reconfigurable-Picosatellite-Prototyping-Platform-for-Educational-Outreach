#ifndef SENDER_CONFIG_H
#define SENDER_CONFIG_H

/*
 * Configuration: network parameters, pin definitions and hardware constants.
 * Copy this file to config.h and fill in your own values.
 *
 * IP requirements:
 * - The two boards and the backend PC must be on the same LAN and subnet.
 * - Give each board a static IP outside the router's DHCP pool.
 * - gateway and subnet must match your router.
 */

#include <IPAddress.h>

// WiFi credentials and receiver (photo upload target)
const char* const ssid         = "YOUR_WIFI_SSID";
const char* const password     = "YOUR_WIFI_PASSWORD";
const char* const receiverIP   = "RECEIVER_IP";   // static IP of the receiver board
const int         receiverPort = 8080;

// Backend WebSocket server
const char* const backendIP   = "BACKEND_IP";     // IP of the PC running code/server
const int         backendPort = 5001;
const char* const backendPath = "/ws";

// Static IP of this board, on the same subnet as the gateway
const IPAddress local_IP(0, 0, 0, 0);
const IPAddress gateway(0, 0, 0, 0);
const IPAddress subnet(255, 255, 255, 0);         // adjust to your network

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
