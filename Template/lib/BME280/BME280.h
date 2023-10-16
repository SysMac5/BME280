/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Höf.:  S. Maggi Snorrason
 *  Netf.:  sms70@hi.is
 *  Dags.:  12. október 2023                                   
 *                                                             
 *     Lýsing:  Haus fyrir BME280.cpp skránna þar sem búið er
 *              að skilgreina BME280 hlutinn sem er tengingin
 *              við BME280 skynjarann.
 * 
 *              Einnig er búið að skilgreina staðföng gistanna
 *              í skynjaranum og allar mögulegarstillingarnar 
 *              á mode og oversampling rate.
 * 
 *              Allar aðgerðir (e. methods) fyrir BME280
 *              hlutinn eru einnig skilgreindar, þ.e. API.
 * 
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#pragma once
#include <hardware/i2c.h>

#define BME280_DEFAULT_I2CADDR      0x76    // SDO tengt við GND.
#define BME280_ALTERNATE_I2CADDR    0x77    // SDO tengt við VDDIO.
#define BME280_CHIP_ID              0x60    // Auðkenni skynjarans.

#define BME280_ID                   0xD0    // Auðkennisgistið.
#define BME280_CMD_RESET            0xE0    // Skipar endurræsun.
#define BME280_CTRL_HUMIDITY        0xF2    // Stýrir umframsýnatöku á rakastigi.
#define BME280_STATUS               0xF3    // Gistið sem geymir stöðuna.
#define BME280_CTRL_MEASURE         0xF4    // Stýrir umframsýnatöku á hitastigi og loftþrýstingi.
#define BME280_CONFIG               0xF5    // Gistið ákvarðar tíðni, síun and viðmót.

// Hrátt loftþrýsingsmæligildi
#define BME280_PRESSURE_MSB         0xF7    // Most Significant Byte
#define BME280_PRESSURE_LSB         0xF8    // Least Significant Byte
#define BME280_PRESSURE_XLSB        0xF9    // Extra Least Significant Byte

// Hrátt hitastigsmæligildi
#define BME280_TEMPERATURE_MSB      0xFA    // Most Significant Byte
#define BME280_TEMPERATURE_LSB      0xFB    // Least Significant Byte
#define BME280_TEMPERATURE_XLSB     0xFC    // Extra Least Significant Byte

// Hrátt rakastigsmæligildi
#define BME280_HUMIDITY_MSB         0xFD    // Most Significant Byte
#define BME280_HUMIDITY_LSB         0xFE    // Least Significant Byte

/**
 * @class BME280
 * @brief Tenging við BME280 skynjara.
 */
class BME280 {
public:
    /** @brief Setur skynjarann á Sleep Mode. */
    static const uint8_t SLEEP_MODE = 0b00;
    /** @brief Setur skynjarann á Forced Mode. */
    static const uint8_t FORCED_MODE = 0b01;
    /** @brief Setur skynjarann á Normal Mode. */
    static const uint8_t NORMAL_MODE = 0b11;

    /** @brief Yfirsöfnun sleppt. */
    static const uint8_t OVERSAMPLING_RATE_SKIPPED = 0b000;
    /** @brief Yfirsafnar sinnum 1. */
    static const uint8_t OVERSAMPLING_RATE_1 = 0b001;
    /** @brief Yfirsafnar sinnum 2. */
    static const uint8_t OVERSAMPLING_RATE_2 = 0b010;
    /** @brief Yfirsafnar sinnum 4. */
    static const uint8_t OVERSAMPLING_RATE_4 = 0b011;
    /** @brief Yfirsafnar sinnum 8. */
    static const uint8_t OVERSAMPLING_RATE_8 = 0b100;
    /** @brief Yfirsafnar sinnum 16. */
    static const uint8_t OVERSAMPLING_RATE_16 = 0b101;

    /**
     * @brief Smiður fyrir BME280 hlut.
     * @param i2c Bendill á I2C vélbúnaðartilvikið sem á að nota.
     * @param addr I2C staðfangið fyrir skynjarann, sjálfstillist á 0x76 sem er SDO tengt við GND.
     */
    BME280(i2c_inst_t* i2c, uint8_t addr = BME280_DEFAULT_I2CADDR) :    _i2c(i2c), 
                                                                        _address(addr),
                                                                        temperature(0),
                                                                        pressure(0),
                                                                        humidity(0) {};

    /**
     * @brief Sjálfgefni smiðurinn fyrir BME280 hlut.
     */
    BME280() {}

    bool init(void);
    bool read(void);

    bool is_connected(void);
    bool set_mode(uint8_t mode, uint8_t oversampling_rate);
    bool reset(void);

    float get_temperature(void);
    float get_pressure(void);
    float get_humidity(void);

private:
    /** @brief I2C vélbúnaðartilvikið sem er notað. */
    i2c_inst_t* _i2c;
    /** @brief I2C staðfangið fyrir skynjarann. */
    uint8_t _address;

    /** @brief Gildin frá nemum skynjarans. */
    float temperature, pressure, humidity;

    struct BME_Comp_Coeff_t {
        uint16_t dig_T1, dig_P1;
        int16_t dig_T2, dig_T3, dig_P2, dig_P3, dig_P4, 
                dig_P5, dig_P6, dig_P7, dig_P8, dig_P9, 
                dig_H2, dig_H4, dig_H5;
        uint8_t dig_H1, dig_H3;
        int8_t dig_H6;  
    } comp_coeffs;
    bool fetch_compensation_data(void);
    void compensate_values( float* temperature,
                            float* pressure,
                            float* humidity,
                            int32_t raw_temperature,
                            int32_t raw_pressure,
                            int32_t raw_humidity);

    bool send_command(uint8_t register_address, uint8_t argument);
    bool send_command(uint8_t register_address);
    bool read_registers(uint8_t register_address, uint8_t register_count, uint8_t *data);
};

static uint8_t crc8(const uint8_t *data, int len);