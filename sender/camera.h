#ifndef SENDER_CAMERA_H
#define SENDER_CAMERA_H

/*
 * Camera module: camera init, live frame capture and single-shot photo capture.
 */

#include <Arduino.h>

/** Initialize the camera; returns true on success */
bool initCamera();

#endif // SENDER_CAMERA_H
