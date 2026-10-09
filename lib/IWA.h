#ifndef IWA_H
#define IWA_H

#include <ArduinoJson.h>

#include "Arduino.h"
#include "Wire.h"

class IWA {
   public:
    IWA();
    bool begin();
    bool getData(float &temperature, float &humidity);
    bool getJSON(JsonDocument &doc);

   private:
    uint8_t _address;
    float _temperature, _humidity;
    bool readSensor_AHTxx();
    bool readSensor_SHT30();
    bool ping(uint8_t address);
    uint8_t _rawData[7] = {0, 0, 0, 0, 0, 0, 0};
    bool useAHT21;
};

#endif  // IWA_H