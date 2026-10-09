#include "ILB.h"

ILB::ILB(int addr) {
    _wire = &Wire;
    i2cAddress = addr;
}

ILB::ILB() {
    _wire = &Wire;
    i2cAddress = ILB_ADDRESS;
}

ILB::ILB(TwoWire *w, int addr) {
    _wire = w;
    i2cAddress = addr;
}

ILB::ILB(TwoWire *w) {
    _wire = w;
    i2cAddress = ILB_ADDRESS;
}

bool ILB::begin() {
    uint8_t part_id = readRegister(ILB_PART_ID);

    if ((part_id >> 4) != 0x0B) {
        return false;
    }

    if (!reset()) {
        return false;
    }

    enable(true);
    if (!enabled()) {
        return false;
    }

    setGain(ILB_GAIN_3);
    setResolution(ILB_RESOLUTION_18BIT);

    return true;
}

bool ILB::getData(float &UVI, float &Lux) {
    setGain(ILB_GAIN_3);                  // Recommended for Lux - x3
    setResolution(ILB_RESOLUTION_18BIT);  // Recommended for Lux - 18-bit
    setMode(ILB_MODE_ALS);
    delay(100);
    while (!newDataAvailable());
    Lux = getLux();

    setGain(ILB_GAIN_18);                 // Recommended for UVI - x18
    setResolution(ILB_RESOLUTION_20BIT);  // Recommended for UVI - 20-bit
    setMode(ILB_MODE_UVS);
    delay(400);
    while (!newDataAvailable());
    UVI = getUVI();

    return true;
}

bool ILB::getJSON(JsonDocument &doc) {
    float UVI, Lux;
    if (!getData(UVI, Lux)) {
        return false;
    }

    JsonArray dataArray = doc["ILB"].to<JsonArray>();

    JsonObject dataSet = dataArray.add<JsonObject>();
    dataSet["name"] = "UVI";
    dataSet["value"] = UVI;
    dataSet["unit"] = "";

    dataSet = dataArray.add<JsonObject>();
    dataSet["name"] = "Lux";
    dataSet["value"] = Lux;
    dataSet["unit"] = "lx";

    return true;
}

/*!
 *  @brief  Perform a soft reset with 10ms delay.
 *  @returns True on success (reset bit was cleared post-write)
 */
bool ILB::reset(void) {
    uint8_t _r = readRegister(ILB_MAIN_CTRL);
    _r |= 0b00010000;
    writeRegister(ILB_MAIN_CTRL, _r);
    delay(10);
    _r = readRegister(ILB_MAIN_CTRL);
    if (_r != 0) {
        return false;
    }

    return true;
}

bool ILB::newDataAvailable(void) {
    uint8_t status = readRegister(ILB_MAIN_STATUS);
    status >>= 3;
    status &= 1;
    return status;
}

/*!
 *  @brief  Read 3 bytes from the ambient data register without checking for new
 *  data; returns up to 20 bits in a 32-bit int.
 */
uint32_t ILB::readALS(void) {
    uint8_t _lsb = readRegister(ILB_ALSDATA_LSB);
    uint8_t _msb = readRegister(ILB_ALSDATA_MSB);
    uint8_t _hsb = readRegister(ILB_ALSDATA_HSB);
    _hsb &= 0x0F;
    uint32_t _out = ((uint32_t)_hsb << 16) | ((uint16_t)_msb << 8) | _lsb;
    return _out;
}

/*!
 *  @brief  Read 3 bytes from the UV data register without checking for new data;
 *  returns up to 20 bits in a 32-bit int.
 */
uint32_t ILB::readUVS(void) {
    uint8_t _lsb = readRegister(ILB_UVSDATA_LSB);
    uint8_t _msb = readRegister(ILB_UVSDATA_MSB);
    uint8_t _hsb = readRegister(ILB_UVSDATA_HSB);
    _hsb &= 0x0F;
    uint32_t _out = ((uint32_t)_hsb << 16) | ((uint16_t)_msb << 8) | _lsb;
    return _out;
}

float ILB::getLux() {
    uint32_t raw = readALS();
    uint8_t _gain = (uint8_t)(getGain());
    uint8_t _resolution = (uint8_t)(getResolution());
    float lux = 0.6 * (float)(raw) / (gain_factor[_gain] * res_factor[_resolution]) * (float)(WFAC);
    return lux;
}

float ILB::getUVI() {
    uint32_t raw = readUVS();
    uint8_t _gain = (uint8_t)(getGain());
    uint8_t _resolution = (uint8_t)(getResolution());
    float uvi = (float)(raw) / ((gain_factor[_gain] / gain_factor[ILB_GAIN_18]) * (res_factor[_resolution] / res_factor[ILB_RESOLUTION_20BIT]) * (float)(UV_SENSITIVITY)) * (float)(WFAC);
    return uvi;
}

void ILB::enable(bool en) {
    uint8_t _r = readRegister(ILB_MAIN_CTRL);
    _r |= (1 << 1);
    writeRegister(ILB_MAIN_CTRL, (uint8_t)_r);
}

bool ILB::enabled(void) {
    uint8_t _r = readRegister(ILB_MAIN_CTRL);
    _r >>= 1;
    _r &= 1;
    return _r;
}

void ILB::setMode(ILB_mode_t mode) {
    uint8_t _r = readRegister(ILB_MAIN_CTRL);
    _r &= 0xF7;
    _r |= ((uint8_t)mode << 3);
    writeRegister(ILB_MAIN_CTRL, (uint8_t)_r);
}

ILB_mode_t ILB::getMode(void) {
    uint8_t _r = readRegister(ILB_MAIN_CTRL);
    _r >>= 3;
    _r &= 1;
    return (ILB_mode_t)_r;
}

void ILB::setGain(ILB_gain_t gain) {
    writeRegister(ILB_GAIN, (uint8_t)gain);
}

ILB_gain_t ILB::getGain(void) {
    uint8_t _r = readRegister(ILB_GAIN);
    _r &= 7;
    return (ILB_gain_t)_r;
}

// Higher resolutions take longer to read (see ILB_resolution_t).
void ILB::setResolution(ILB_resolution_t res) {
    uint8_t _r = 0;
    _r |= (res << 4);
    writeRegister(ILB_MEAS_RATE, (uint8_t)_r);
}

ILB_resolution_t ILB::getResolution(void) {
    uint8_t _r = readRegister(ILB_MEAS_RATE);
    _r &= 0x70;
    _r = 7 & (_r >> 4);
    return (ILB_resolution_t)_r;
}

// Readings below `lower` or above `higher` fire the interrupt.
void ILB::setThresholds(uint32_t lower, uint32_t higher) {
    uint8_t _r = higher;
    writeRegister(ILB_THRESH_UP, _r);
    _r = higher >> 8;
    writeRegister(ILB_THRESH_UP + 1, _r);
    _r = higher >> 16;
    _r &= 0x0F;
    writeRegister(ILB_THRESH_UP + 2, _r);
    _r = lower;
    writeRegister(ILB_THRESH_LOW, _r);
    _r = lower >> 8;
    writeRegister(ILB_THRESH_LOW + 1, _r);
    _r = lower >> 16;
    _r &= 0x0F;
    writeRegister(ILB_THRESH_LOW + 2, _r);
}

// Arm the interrupt against the setThresholds() limits; persistance = number of
// consecutive out-of-range readings before firing (0 = every reading fires).
void ILB::configInterrupt(bool enable, ILB_mode_t source,
                          uint8_t persistance) {
    uint8_t _r = 0;
    _r |= (enable << 2) | (1 << 4) | (source << 5);
    writeRegister(ILB_INT_CFG, _r);
    if (persistance > 0x0F) persistance = 0x0F;
    uint8_t _p = 0;
    _p |= persistance << 4;
    writeRegister(ILB_INT_PST, _p);
}

uint8_t ILB::writeRegister(uint8_t reg, uint8_t val) {
    _wire->beginTransmission(i2cAddress);
    _wire->write(reg);
    _wire->write(val);
    return _wire->endTransmission();
}

uint8_t ILB::readRegister(uint8_t reg) {
    uint8_t regValue = 0;
    _wire->beginTransmission(i2cAddress);
    _wire->write(reg);
    _wire->endTransmission();
    _wire->requestFrom(i2cAddress, 1);
    if (_wire->available()) {
        regValue = _wire->read();
    }
    return regValue;
}
