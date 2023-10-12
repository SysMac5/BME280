#include "BME280.h"
#include <string.h> // Used for memcpy

/*
 * TO-DO:
 *  - Complete the init code
 *  - Complete the read code
 *  - Implement any added methods
*/

/// @brief Initialize communications for the BME280 sensor
/// @return True if successful, false if not
bool BME280::init(void) {
    /* Reset the sensor? */

    bool successful = true;
    successful &= fetchCompensationData(); // If fetching worked then successful stays true, otherwise it's set to false
    sleep_us(250); // Wait a bit to allow the sensor to finish processing

    /* Insert other methods to call as part of setting up communications */
    /* Good idea to set the sensor to normal mode and set oversampling rates to something other than 0 (which skips measurements) */

    return successful;
}

/// @brief Read temperature, pressure and relative humidity from the BME280
/// @return True if successful, false if not
bool BME280::read() {
    /* Read raw bytes from sensor*/
    
    int32_t raw_t, raw_p, raw_h;

    /* Insert the raw bytes into the respective variables using bitwise OR (|) and bit shifting (<< and >>) */
    /* See the SCD30 read function to see how it's possible to shift and bitwise OR to assemble an int32_t from multiple bytes */
    /* Note the sensor is big endian, so the values that are read out first (lowest address) are the most significant bits */

    float temperature, pressure, humidity;
    compensateValues(&temperature, &pressure, &humidity, raw_t, raw_p, raw_h);

    /* Do something with the values, use getter methods? Insert them into public variables? */

    return true;
}


/********** Private methods **********/


/// @brief Type pun a uint16_t variable to an int16_t variable
/// @param x Input value in uint16_t format
/// @return A reinterpreted value containing the original data in int16_t format
[[nodiscard]] int16_t uint16_t_to_int16_t(uint16_t x) noexcept {
    return *reinterpret_cast<int16_t*>(&x);
}

/// @brief Type pun a uint8_t variable to an int8_t variable
/// @param x Input value in uint8_t format
/// @return A reinterpreted value containing the original data in int8_t format
[[nodiscard]] int8_t uint8_t_to_int8_t(uint8_t x) noexcept {
    return *reinterpret_cast<int8_t*>(&x);
}

/// @brief Fetch the compensation data from the BME280
/// @return True of fetching successful, false if not
bool BME280::fetchCompensationData(void) {
    // We want to read the full compensation register data
    uint8_t buffer[33]{0};

    // Two reads because the calibration/compensation registers are not aligned
    uint8_t compensation_reg_first = 0x88;
    uint8_t compensation_reg_second = 0xE1;
    if (i2c_write_timeout_us(_i2c, _address, &compensation_reg_first, 1, false, 10000) != 1) return false;
    if (i2c_read_timeout_us(_i2c, _address, buffer, 26, false, 10000) != 26) return false;

    if (i2c_write_timeout_us(_i2c, _address, &compensation_reg_second, 1, false, 10000) != 1) return false;
    if (i2c_read_timeout_us(_i2c, _address, buffer + 26, 7, false, 10000) != 7) return false;

    // Zero-initialize the comp_coeffs struct
    memset(&comp_coeffs, 0, sizeof(comp_coeffs));

    // Insert the compensation data into the struct for it
    comp_coeffs.dig_T1 = ((uint16_t)buffer[1] << 8) | (uint16_t)buffer[0];
    comp_coeffs.dig_T2 = uint16_t_to_int16_t(((uint16_t)buffer[3] << 8) | (uint16_t)buffer[2]);
    comp_coeffs.dig_T3 = uint16_t_to_int16_t(((uint16_t)buffer[5] << 8) | (uint16_t)buffer[4]);
    comp_coeffs.dig_P1 = ((uint16_t)buffer[7] << 8) | (uint16_t)buffer[6];
    comp_coeffs.dig_P2 = uint16_t_to_int16_t(((uint16_t)buffer[9] << 8) | (uint16_t)buffer[8]);
    comp_coeffs.dig_P3 = uint16_t_to_int16_t(((uint16_t)buffer[11] << 8) | (uint16_t)buffer[10]);
    comp_coeffs.dig_P4 = uint16_t_to_int16_t(((uint16_t)buffer[13] << 8) | (uint16_t)buffer[12]);
    comp_coeffs.dig_P5 = uint16_t_to_int16_t(((uint16_t)buffer[15] << 8) | (uint16_t)buffer[14]);
    comp_coeffs.dig_P6 = uint16_t_to_int16_t(((uint16_t)buffer[17] << 8) | (uint16_t)buffer[16]);
    comp_coeffs.dig_P7 = uint16_t_to_int16_t(((uint16_t)buffer[19] << 8) | (uint16_t)buffer[18]);
    comp_coeffs.dig_P8 = uint16_t_to_int16_t(((uint16_t)buffer[21] << 8) | (uint16_t)buffer[20]);
    comp_coeffs.dig_P9 = uint16_t_to_int16_t(((uint16_t)buffer[23] << 8) | (uint16_t)buffer[22]);
    comp_coeffs.dig_H1 = buffer[25];
    comp_coeffs.dig_H2 = uint16_t_to_int16_t(((uint16_t)buffer[27] << 8) | (uint16_t)buffer[26]);
    comp_coeffs.dig_H3 = buffer[28];
    comp_coeffs.dig_H4 = uint16_t_to_int16_t(((uint16_t)buffer[29] << 4) | (uint16_t)buffer[30] & 0x0F);
    comp_coeffs.dig_H5 = uint16_t_to_int16_t((((uint16_t)buffer[30] & 0xF0 ) >> 4) | ((uint16_t)buffer[31] << 4));
    comp_coeffs.dig_H6 = uint8_t_to_int8_t(buffer[32]);
    return true;
}

/// @brief Do the compensation calculation for temperature, pressure and humidity.
/// Note that pressure and humidity rely on temperature to stay accurate.
/// @param temperature Pointer to a float where the temperature value will be inserted (in °C)
/// @param pressure Pointer to a float where the pressure value will be inserted (in hPa)
/// @param humidity Pointer to a float where the humidity value will be inserted (in %RH)
/// @param raw_temperature Raw temperature reading from BME280 sensor
/// @param raw_pressure Raw pressure reading from BME280 sensor
/// @param raw_humidity Raw humidity reading from BME280 sensor
void BME280::compensateValues(  float* temperature,
                                float* pressure,
                                float* humidity,
                                int32_t raw_temperature,
                                int32_t raw_pressure,
                                int32_t raw_humidity) {
    // Temperature compensation
    int32_t t_fine;
    int32_t var1, var2, T;
    var1 = ((((raw_temperature >> 3) - ((int32_t)comp_coeffs.dig_T1 << 1))) * ((int32_t)comp_coeffs.dig_T2)) >> 11;
    var2 = (((((raw_temperature >> 4) - ((int32_t)comp_coeffs.dig_T1)) * ((raw_temperature >> 4) - ((int32_t)comp_coeffs.dig_T1))) >> 12) * ((int32_t)comp_coeffs.dig_T3)) >> 14;
    t_fine = var1 + var2;
    T = (t_fine * 5 + 128) >> 8;
    *temperature = (float)T / 100.0f;

    // Pressure compensation
    int64_t p;
    var1 = 0;
    var2 = 0;
    var1 = ((int64_t)t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)comp_coeffs.dig_P6;
    var2 = var2 + ((var1 * (int64_t)comp_coeffs.dig_P5) << 17);
    var2 = var2 + (((int64_t)comp_coeffs.dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)comp_coeffs.dig_P3) >> 8) + ((var1 * (int64_t)comp_coeffs.dig_P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)comp_coeffs.dig_P1) >> 33;
    if (var1 == 0) {
        *pressure = 0.0f;
    } else {
        p = 1048576 - raw_pressure;
        p = (((p << 31) - var2) * 3125) / var1;
        var1 = (((int64_t)comp_coeffs.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
        var2 = (((int64_t)comp_coeffs.dig_P8) * p) >> 19;
        p = ((p + var1 + var2) >> 8) + (((int64_t)comp_coeffs.dig_P7) << 4);
        *pressure = (float)((uint32_t)p) / 25600.0f;
    }

    // Humidity compensation
    int32_t h_temp;
    h_temp = (t_fine - ((int32_t)76800));
    h_temp = (((((raw_humidity << 14) - (((int32_t)comp_coeffs.dig_H4) << 20) - (((int32_t)comp_coeffs.dig_H5) * h_temp)) + ((int32_t)16384)) >> 15) * (((((((h_temp * ((int32_t)comp_coeffs.dig_H6)) >> 10) * (((h_temp * ((int32_t)comp_coeffs.dig_H3)) >> 11) + ((int32_t)32768))) >> 10) + ((int32_t)2097152)) * ((int32_t)comp_coeffs.dig_H2) + 8192) >> 14));
    h_temp = (h_temp - (((((h_temp >> 15) * (h_temp >> 15)) >> 7) * ((int32_t)comp_coeffs.dig_H1)) >> 4));
    h_temp = (h_temp < 0 ? 0 : h_temp);
    h_temp = (h_temp > 419430400 ? 419430400 : h_temp);
    *humidity = (float)((uint32_t)(h_temp >> 12)) / 1024.0f;
}