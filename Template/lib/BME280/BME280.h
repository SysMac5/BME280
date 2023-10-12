#pragma once
#include <hardware/i2c.h>
/* 
 * TO-DO:
 *  - Add memory map using #define or const uint8_t
 *  - Add methods not present (for example reading from and writing to registers, checking if connected, set mode to sleep, normal or forced...)
 *  - Implement methods in BME280.cpp
 * 
 * Notes:
 *  - The BME280 always starts up in sleep state with the oversampling registers set to 0 (all measurements disabled)
 *  - The pressure and humidity measurements need a temperature measurement to compensate for temperature effects
 *  - It's a good idea to reset the BME280 in the init function and then wait ~5-10 ms for it to finish resetting
*/

#define BME280_DEFAULT_I2CADDR 0x76 // SDO connected to GND
#define BME280_ALTERNATE_I2CADDR 0x77 // SDO connected to VDDIO
#define BME280_CHIP_ID 0x60

/* Insert memory map here or inside the class (note, don't put #define inside classes) */

class BME280 {
public:
    /// @brief Construct a new BME280 object
    /// @param i2c Pointer to the I2C hardware instance to be used
    /// @param addr The I2C address for the sensor, default is 0x76 (SDO connected to GND)
    BME280(i2c_inst_t* i2c, uint8_t addr = BME280_DEFAULT_I2CADDR) : _i2c(i2c), _address(addr) {};
    BME280() {}; // Default constructor

    bool init(void);
    bool read(/* This can be a void if using getter functions or a pointer to where the values will be placed */);

    /* Insert other methods like checking if connected, setting modes, changing oversampling rate */

private:
    i2c_inst_t* _i2c;
    uint8_t _address;

    // Code necessary to get proper values
    struct BME_Comp_Coeff_t {
        uint16_t dig_T1, dig_P1;
        int16_t dig_T2, dig_T3, dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9, dig_H2, dig_H4, dig_H5;
        uint8_t dig_H1, dig_H3;
        int8_t dig_H6;  
    } comp_coeffs;
    bool fetchCompensationData(void);
    void compensateValues(  float* temperature,
                            float* pressure,
                            float* humidity,
                            int32_t raw_temperature,
                            int32_t raw_pressure,
                            int32_t raw_humidity);

    /* Insert methods to read from and write to registers*/
};