#ifndef SENDER_WIFI_MANAGER_H
#define SENDER_WIFI_MANAGER_H

/*
 * WiFi manager: establishes and maintains the WiFi connection.
 */

/** Connect with the static IP from config; blocks until connected or timed out */
void connectToWiFi();

#endif // SENDER_WIFI_MANAGER_H
