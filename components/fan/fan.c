#include "driver/gpio.h"
#include "fan.h"
#include "esp_log.h"

#define FAN1 GPIO_NUM_25
#define FAN2 GPIO_NUM_26

const uint8_t BOX_VENT = FAN1;
const uint8_t LAMP_VENT = FAN2;

const static char* TAG = "FAN";



void fanInit() {

    gpio_reset_pin(BOX_VENT);
    gpio_reset_pin(LAMP_VENT);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(BOX_VENT, GPIO_MODE_OUTPUT);
    gpio_set_direction(LAMP_VENT, GPIO_MODE_OUTPUT);

}

void fanTurnON(uint8_t fan ) {
    if(!gpio_get_level(fan)){
        gpio_set_level(fan, 1);
        ESP_LOGI(TAG, "ON Fan: %d", fan);
    }
}

void fanTurnOFF(uint8_t fan) {
    if(gpio_get_level(fan)){
        gpio_set_level(fan, 0);
        ESP_LOGI(TAG, "OFF Fan: %d", fan);
    }

}