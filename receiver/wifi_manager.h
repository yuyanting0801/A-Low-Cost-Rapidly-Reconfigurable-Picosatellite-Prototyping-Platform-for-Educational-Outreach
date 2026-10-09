#ifndef RECEIVER_WIFI_MANAGER_H
#define RECEIVER_WIFI_MANAGER_H

/*
 * WiFi manager: establishes and maintains the WiFi connection.
 */

/** Connect using the static IP from config.h; blocks until connected or timed out */
void connectToWiFi();

#endif // RECEIVER_WIFI_MANAGER_H
