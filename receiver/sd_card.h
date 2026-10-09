#ifndef RECEIVER_SD_CARD_H
#define RECEIVER_SD_CARD_H

/*
 * SD card module: init and directory creation.
 */

#include <Arduino.h>

/** Init SD card and create the required directories */
void initSDCard();

/** JSON list of photos in /images */
String getPhotoListJson();

/** JSON list of sensor data files in /sensor_data */
String getSensorDataListJson();

#endif // RECEIVER_SD_CARD_H
