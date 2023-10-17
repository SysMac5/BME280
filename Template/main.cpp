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

    sleep_ms(1000);

    printf("[HÖFUNDUR]\n");
    printf("  S. Maggi Snorrason\n");
    printf("  sms70@hi.is\n\n");

    sleep_ms(1000);

    printf("[UPPLÝSINGAR]\n");
    printf("  Skynjari: BME280 - Combined humidity and pressure sensor\n");
    printf("  I2C ræsing með Actual set baudrate %d Hz \n", i2c_init(i2c0, 400000));
    printf("  I2C HW block notað: %d \n", 0);
    printf("  PIN_BMW_SDA = %d \n", PIN_BME_SDA);
    printf("  PIN_BMW_SCL = %d \n", PIN_BME_SCL);
    printf("  I2C staðfangið = 0x%02x \n\n", BME280_DEFAULT_I2CADDR);
    
    gpio_set_function(PIN_BME_SDA, GPIO_FUNC_I2C);
    gpio_set_function(PIN_BME_SCL, GPIO_FUNC_I2C);
    
    bme = BME280(i2c0, BME280_DEFAULT_I2CADDR);

    sleep_ms(1000);
    
    if(!bme.init()) {
        printf("[VILLA] Upphafsstilling misstókst!\n");
        while(true);
    } else {
        printf("[KEYRSLA]\n  Upphafsstilling tókst.\n\n");
    }

    sleep_ms(1000);
}

void loop() {
    if (bme.is_connected()) {
        bme.read();

        printf("[AFLESTUR]\n");
        printf("       Hitastig :  %.2f °C            \n", bme.get_temperature());
        printf(" Loftþrýstingur :  %.2f hPa           \n", bme.get_pressure());
        printf("       Rakastig :  %.1f%%             \n", bme.get_humidity());
    } else {
        printf("[AFLESTUR]\n");
        printf("       Hitastig :  - °C               \n");
        printf(" Loftþrýstingur :  - hPa              \n");
        printf("       Rakastig :  -%%                \n\n");
        printf("[VILLA] Skynjari aftengdur.\n");
        while(true);
    }
    
    sleep_ms(1000);

    printf("\033[F\033[F\033[F\033[F");
}

int main() {
    init();
    while(1) {
        loop();
    }
    return 0;
}