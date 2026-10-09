#ifndef SENDER_WEB_SERVER_H
#define SENDER_WEB_SERVER_H

/*
 * HTTP server module: local endpoints to trigger a capture, fetch sensor data and stream video.
 */

/** HTTP task: registers the routes and serves client requests */
void commandTask(void* pvParameters);

#endif // SENDER_WEB_SERVER_H
