#include "BME280.h"
#include <string.h> // Used for memcpy

/**
 * @brief Upphafsstilla samskipti við skynjarann.
 * @return True ef upphafsstilling tókst, annars false.
 */
bool BME280::init(void) {
    if (!reset()) return false;

    temperature = pressure = humidity = 0; //kannski má sleppa þessu

    bool successful = true;
    successful &= fetch_compensation_data();
    sleep_us(250);

    successful &= set_mode(NORMAL_MODE, OVERSAMPLING_RATE_16);

    return successful;
}

/**
 * @brief Les hitastig, loftþrýsing og rakastig frá skynjaranum.
 * @return True ef aflestur tekst, annars false.
 */
bool BME280::read() {
    uint8_t* data_in_bytes = read_registers(BME280_PRESSURE_MSB, 8);
    
    int32_t raw_temperature, raw_pressure, raw_humidity;

    raw_pressure = (data_in_bytes[0] | data_in_bytes[1] | (data_in_bytes[2] << 4));
    raw_temperature = (data_in_bytes[3] | data_in_bytes[4] | (data_in_bytes[5] << 4));
    raw_humidity = (data_in_bytes[6] | data_in_bytes[7]);

    compensate_values(  &temperature, 
                        &pressure, 
                        &humidity, 
                        raw_temperature, 
                        raw_pressure, 
                        raw_humidity);

    return true;
}

/**
 * @brief Athugar ef skynjarinn er tengdur.
 * @return True ef skynjarinn er tengdur, annars false.
 */
bool BME280::is_connected(void) {
    uint8_t* data_in_bytes = read_registers(BME280_ID, 1);
    return (data_in_bytes[0] | data_in_bytes[1]) == BME280_CHIP_ID;
}

/**
 * @brief Stillir aflestur nemanna í skynjaranum.
 * @param mode Tveir bitar sem segja til um hvort skynjarinn sé í Sleep Mode (mode=00), 
 *        Forced Mode (mode=01 eða mode=10) eða Normal Mode (mode=11).
 * @param oversampling_rate Þrír bitar sem segja til um hvort og hversu mikið skynjarinn
 *        yfirsafnar gildunum frá nemunum. Bitarnir 000 eru Skip, 001 er yfirsögnun sinnum 1,
 *        010 er sinnum 2, 011 er sinnum 4, 100 er sinnum 8 og 101 er sinnum 16.
 * @return True ef stilling tókst, annars false.
 */
bool BME280::set_mode(uint8_t mode, uint8_t oversampling_rate) {
    if (mode < 0b00 || mode > 0b11) return false;
    if (oversampling_rate < 0b000 || oversampling_rate > 0b101) return false;

    bool success = true;
    success &= send_command(BME280_CTRL_HUMIDITY, oversampling_rate & 0b111);
    success &= send_command(BME280_CTRL_MEASURE,
                            (oversampling_rate & 0b111) | (oversampling_rate & 0b111) | (mode & 0b11));

    return success;
}

/**
 * @brief Endurræsir skynjarann.
 * @return True ef endurræsing tekst, annars false.
 */
bool BME280::reset(void) {
    if (!send_command(BME280_CMD_RESET, 0xB6)) return false;
    sleep_ms(10);
    return true;
}


/********** Getters **********/

/**
 * @brief Skilar hitastig frá skynjara.
 * @return Hitastigið í Celsíus [°C].
 */
float BME280::get_temperature(void) {
    return temperature;
}

/**
 * @brief Skilar loftþrýstinginn frá skynjara.
 * @return Loftþrýstingur í hektópaskal [hPa].
 */
float BME280::get_pressure(void) {
    return pressure;
}

/**
 * @brief Skilar rakastiginu frá skynjara.
 * @return Rakastigið sem hlutfallslegur raki [%]
 */
float BME280::get_humidity(void) {
    return humidity;
}


/********** Private methods **********/

/**
 * @brief Sendir skipun á skynjarann.
 * @param command Skipunargistið, 2 bæti að lengd.
 * @param argument Inngildi skipunarinnar, 2 bæti að lengd.
 * @return True ef skipun tekst, annars false.
 */
bool BME280::send_command(uint16_t command, uint16_t argument) {
    uint8_t buffer[5];
    buffer[0] = (command >> 8) & 0xFF;
    buffer[1] = command & 0xFF;
    buffer[2] = argument >> 8;
    buffer[3] = argument & 0xFF;
    buffer[4] = crc8(buffer + 2, 2);

    return (i2c_write_timeout_us(_i2c, _address, buffer, 5, false, 10000) == 5);
}

/**
 * @brief Sendir skipun á skynjarann.
 * @param command Skipunargistið, 2 bæti að lengd.
 * @return True ef skipun tekst, annars false.
 */
bool BME280::send_command(uint16_t command) {
    uint8_t buffer[2];
    buffer[0] = (command >> 8) & 0xFF;
    buffer[1] = command & 0xFF;
    
    return (i2c_write_timeout_us(_i2c, _address, buffer, 2, false, 10000) == 2);
}

/**
 * @brief Les úr gistum skynjara.
 * @param register_address Staðfang fyrsta gistisins, 2 bæti að lengd.
 * @param register_count Fjöldi gista sem á að lesa.
 * @return Bætafylki með gildum úr fyrsta gistinu og (register_count - 1) næstu gistum.
 */
uint8_t* BME280::read_registers(uint16_t register_address, uint16_t register_count) {
    send_command(register_address);
    sleep_ms(5);

    size_t len = register_count * 2;
    uint8_t buffer[len];

    i2c_read_timeout_us(_i2c, _address, buffer, len, false, 10000);

    return buffer;
}







/*
 *  Allt hér fyrir neðan var ekki forritað af nemanda.
 */

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
bool BME280::fetch_compensation_data(void) {
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
void BME280::compensate_values(  float* temperature,
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

///@brief Calculate CRC8 checksum
///@param *data
///       Pointer to data to calculate checksum for
///@param len
///       Length of data in bytes
///@return Checksum byte
static uint8_t crc8(const uint8_t *data, int len) {
    const uint8_t POLYNOMIAL(0x31);
    uint8_t crc(0xFF);

    for(int j = len; j; --j) {
        crc ^= *data++;

        for(int i = 8; i; --i) {
            crc = (crc & 0x80) ? (crc << 1) ^ POLYNOMIAL : (crc << 1);
        }
    }
    return crc;
}