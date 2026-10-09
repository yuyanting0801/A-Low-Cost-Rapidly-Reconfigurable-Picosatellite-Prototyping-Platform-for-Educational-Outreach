#include "IWA.h"

IWA::IWA() {
}

bool IWA::ping(uint8_t addr) {
    int8_t ret = 0;
    Wire.beginTransmission(addr);
    ret = Wire.endTransmission();
    if (ret != 0)
        return false;
    return true;
}

bool IWA::begin() {
    if (ping(0x38)) {
        useAHT21 = true;
        _address = 0x38;
        // AHT21 detected
        return true;
    } else if (ping(0x44)) {
        useAHT21 = false;
        _address = 0x44;
        // SHT30 detected
        return true;
    } else {
        return false;
    }
}

bool IWA::readSensor_SHT30() {
    Wire.beginTransmission(_address);
    Wire.write(0x2C);
    Wire.write(0x06);
    if (Wire.endTransmission(false) != 0)
        return false;

    delay(500);  // Measurement duration

    Wire.requestFrom(_address, (uint8_t)6);

    uint16_t rawTemperature = (Wire.read() << 8) | Wire.read();
    Wire.read();
    uint16_t rawHumidity = (Wire.read() << 8) | Wire.read();
    Wire.read();

    _temperature = 175.0 * rawTemperature / 65535.0 - 45.0;
    _humidity = 100.0 * rawHumidity / 65535.0;

    return true;
}

bool IWA::readSensor_AHTxx() {
    Wire.beginTransmission(_address);
    Wire.write(0xAC);  // send measurement command, start measurement
    Wire.write(0x33);  // send measurement control
    Wire.write(0x00);  // send measurement NOP control

    if (Wire.endTransmission(true) != 0) {
        return false;
    }
    uint8_t dataSize = 7;

    Wire.requestFrom(_address, dataSize, (uint8_t) true);  // read into Wire's rxBuffer, stop after transmission

    if (Wire.available() != dataSize) {
        return false;
    }

    for (uint8_t i = 0; i < dataSize; i++) {
        _rawData[i] = Wire.read();
    }

    uint32_t temperature = _rawData[3] & 0x0F;  // 20-bit raw temperature data
    temperature <<= 8;
    temperature |= _rawData[4];
    temperature <<= 8;
    temperature |= _rawData[5];

    _temperature = ((float)temperature / 0x100000) * 200 - 50;

    uint32_t humidity = _rawData[1];  // 20-bit raw humidity data
    humidity <<= 8;
    humidity |= _rawData[2];
    humidity <<= 4;
    humidity |= _rawData[3] >> 4;

    if (humidity > 0x100000) {
        humidity = 0x100000;
    }  // cap RH at 100%; no lower-bound check needed, humidity is unsigned

    _humidity = ((float)humidity / 0x100000) * 100;

    return true;
}

bool IWA::getData(float &temp, float &humi) {
    if (useAHT21) {
        readSensor_AHTxx();
    } else {
        readSensor_SHT30();
    }

    temp = _temperature;
    humi = _humidity;
    return true;
}

bool IWA::getJSON(JsonDocument &doc) {
    if (useAHT21) {
        readSensor_AHTxx();
    } else {
        readSensor_SHT30();
    }

    JsonArray dataArray = doc["IWA"].to<JsonArray>();

    JsonObject dataSet = dataArray.add<JsonObject>();
    dataSet["name"] = "Temperature";
    dataSet["value"] = _temperature;
    dataSet["unit"] = "ºC";

    dataSet = dataArray.add<JsonObject>();
    dataSet["name"] = "Humidity";
    dataSet["value"] = _humidity;
    dataSet["unit"] = "%RH";

    return true;
}
