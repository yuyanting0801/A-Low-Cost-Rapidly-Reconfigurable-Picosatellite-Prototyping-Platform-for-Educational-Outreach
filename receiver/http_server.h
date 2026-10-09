#ifndef RECEIVER_HTTP_SERVER_H
#define RECEIVER_HTTP_SERVER_H

/*
 * HTTP file server: read-only access to stored photos and sensor data.
 */

/** HTTP server task: serve /latest_photo, /photos, /sensor_data on port 80 */
void httpServerTask(void* pvParameters);

#endif // RECEIVER_HTTP_SERVER_H
