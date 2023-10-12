/*
 *  Title: Template
 */

// Includes
#include <stdio.h>
#include <pico/stdlib.h>
#include <hardware/i2c.h>
#include <BME280.h>

// Defines

// Pinouts
const uint8_t PIN_BME_SDA = /* SDA pin here */;
const uint8_t PIN_BME_SCL = /* SCL pin here */;

// Constructors
BME280 bme;

// Global variables and data structures

// Forward declarations

void init() {
    stdio_init_all();
    i2c_init(i2c1, 400000);
    gpio_set_function(PIN_BME_SDA, GPIO_FUNC_I2C);
    gpio_set_function(PIN_BME_SCL, GPIO_FUNC_I2C);
    bme = BME280(/*i2c0 or i2c1*/, /*i2c address (0x76 or 0x77)*/);

    sleep_ms(5000);
    
    printf("Initializing BME280... ");
    if(!bme.init()) {
        printf("Failed to initialize!\n");
        while(true); // Stop here
    } else {
        printf("Initialized.\n");
    }
}

void loop() {
    /*Read from BME and print results*/
}

int main() {
    init();
    while(1) {
        loop();
    }
    return 0;
}