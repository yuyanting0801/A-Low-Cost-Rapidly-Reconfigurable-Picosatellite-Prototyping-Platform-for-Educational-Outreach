#ifndef ILB_H
#define ILB_H

#include <ArduinoJson.h>
#include <Wire.h>
#define UV_SENSITIVITY 1400

#define WFAC 1
/*
 *  Window factor compensates light loss for a device behind tinted/coated glass
 *  (lower transmission at 400-600nm): 1 = clear glass or no window, >1 = tinted
 *  glass (calibrate under a white LED).
 */

#define ILB_ADDRESS 0x53      ///< I2C address
#define ILB_MAIN_CTRL 0x00    ///< Main control register
#define ILB_MEAS_RATE 0x04    ///< Resolution and data rate
#define ILB_GAIN 0x05         ///< ALS and UVS gain range
#define ILB_PART_ID 0x06      ///< Part id/revision register
#define ILB_MAIN_STATUS 0x07  ///< Main status register
#define ILB_ALSDATA_LSB 0x0D  ///< ALS data lowest byte
#define ILB_ALSDATA_MSB 0x0E  ///< ALS data middle byte
#define ILB_ALSDATA_HSB 0x0F  ///< ALS data highest byte
#define ILB_UVSDATA_LSB 0x10  ///< UVS data lowest byte
#define ILB_UVSDATA_MSB 0x11  ///< UVS data middle byte
#define ILB_UVSDATA_HSB 0x12  ///< UVS data highest byte
#define ILB_INT_CFG 0x19      ///< Interrupt configuration
#define ILB_INT_PST 0x1A      ///< Interrupt persistance config
#define ILB_THRESH_UP 0x21    ///< Upper threshold, low byte
#define ILB_THRESH_LOW 0x24   ///< Lower threshold, low byte

typedef enum {
    ILB_MODE_ALS,
    ILB_MODE_UVS,
} ILB_mode_t;

typedef enum {
    ILB_GAIN_1 = 0,
    ILB_GAIN_3,
    ILB_GAIN_6,
    ILB_GAIN_9,
    ILB_GAIN_18,
} ILB_gain_t;

typedef enum {
    ILB_RESOLUTION_20BIT,
    ILB_RESOLUTION_19BIT,
    ILB_RESOLUTION_18BIT,
    ILB_RESOLUTION_17BIT,
    ILB_RESOLUTION_16BIT,
    ILB_RESOLUTION_13BIT,
} ILB_resolution_t;

class ILB {
   public:
    ILB(int addr);
    ILB();
    ILB(TwoWire *w, int addr);
    ILB(TwoWire *w);
    bool begin();
    bool getData(float &Lux, float &UVI);
    bool getJSON(JsonDocument &doc);

    bool reset(void);

    void enable(bool en);
    bool enabled(void);

    void setMode(ILB_mode_t mode);
    ILB_mode_t getMode(void);

    void setGain(ILB_gain_t gain);
    ILB_gain_t getGain(void);

    void setResolution(ILB_resolution_t res);
    ILB_resolution_t getResolution(void);

    void setThresholds(uint32_t lower, uint32_t higher);

    void configInterrupt(bool enable, ILB_mode_t source,
                         uint8_t persistance = 0);

    bool newDataAvailable(void);
    uint32_t readUVS(void);
    uint32_t readALS(void);

    float getLux(void);

    float getUVI(void);

    uint8_t writeRegister(uint8_t reg, uint8_t val);
    uint8_t readRegister(uint8_t reg);

   private:
    TwoWire *_wire;
    int i2cAddress;
    float gain_factor[5] = {1, 3, 6, 9, 18};
    float res_factor[6] = {4, 2, 1, 0.5, 0.25, 0.03125};
};

#endif  // ILB_H