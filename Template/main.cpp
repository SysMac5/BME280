/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Höf.:  S. Maggi Snorrason
 *  Netf.:  sms70@hi.is
 *  Dags.:  12. október 2023                                   
 *                                                             
 *     Lýsing:  
 *                        
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <stdio.h>
#include <pico/stdlib.h>
#include <hardware/i2c.h>
#include <BME280.h>

const uint8_t PIN_BME_SDA = 19;
const uint8_t PIN_BME_SCL = 20;

BME280 bme;

void init() {
    stdio_init_all();
    time_init();
    i2c_init(i2c1, 400000);
    gpio_set_function(PIN_BME_SDA, GPIO_FUNC_I2C);
    gpio_set_function(PIN_BME_SCL, GPIO_FUNC_I2C);
    bme = BME280(i2c1, BME280_ALTERNATE_I2CADDR);

    sleep_ms(5000);
    
    printf("[UPPL.] Upphafsstillir BME280... ");
    if(!bme.init()) {
        printf("[UPPL.] Upphafsstilling misstókst!\n");
        while(true); // Stop here
    } else {
        printf("[UPPL.] Upphafsstillt.\n");
    }
}

void loop() {
    /*Read from BME and print results*/

    printf("\n[UPPL.] Sækir gögn til BME280.\n");

    if (bme.is_connected) bme.read();

    printf("Hitastig: %f °C \n", bme.get_temperature);
    printf("Loftþrýstingur: %f hPa \n", bme.get_pressure);
    printf("Rakastig: %f% \n", bme.get_humidit);
}

int main() {
    init();
    while(1) {
        loop();
    }
    return 0;
}