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

const uint8_t PIN_BME_SDA = 4;
const uint8_t PIN_BME_SCL = 5;

BME280 bme;

void init() {

    stdio_init_all();

    sleep_ms(5000);

    printf("[UPPL.] I2C ræsing með Actual set baudrate %d Hz \n", i2c_init(i2c0, 400000));
    printf("[UPPL.] I2C HW block notað: %d \n", 0);
    printf("[UPPL.] PIN_BMW_SDA = %d \n", PIN_BME_SDA);
    printf("[UPPL.] PIN_BMW_SCL = %d \n", PIN_BME_SCL);
    printf("[UPPL.] I2C staðfangið = 0x%02x \n", BME280_DEFAULT_I2CADDR);
    
    gpio_set_function(PIN_BME_SDA, GPIO_FUNC_I2C);
    gpio_set_function(PIN_BME_SCL, GPIO_FUNC_I2C);
    
    bme = BME280(i2c0, BME280_DEFAULT_I2CADDR);

    sleep_ms(5000);
    
    printf("[UPPL.] Upphafsstillir BME280... \n");
    if(!bme.init()) {
        printf("[VILLA] Upphafsstilling misstókst!\n");
        while(true); // Stop here
    } else {
        printf("[UPPL.] Upphafsstillt.\n\n");
    }
}

void loop() {
    if (bme.is_connected()) bme.read();
    /*
    printf("       Hitastig :  %.2f °C            \n", bme.get_temperature());
    printf(" Loftþrýstingur :  %.2f hPa           \n", bme.get_pressure());
    printf("       Rakastig :  %.1f%%             \n", bme.get_humidity());
    */
    sleep_ms(1000);

    //printf("\033[F\033[F\033[F");
}

int main() {
    init();
    while(1) {
        loop();
    }
    return 0;
}