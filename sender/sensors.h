#ifndef SENDER_SENSORS_H
#define SENDER_SENSORS_H

/*
 * Sensor module: init and readout of the I2C temperature/humidity, light/UV and accelerometer sensors.
 */

#include <Arduino.h>
#include "globals.h"

/** Initialize all I2C sensors; returns true only if all of them responded */
bool initSensors();

/** Read all sensors under the I2C mutex; returns true on success */
bool safeReadSensorData(SensorData& data);

#endif // SENDER_SENSORS_H
