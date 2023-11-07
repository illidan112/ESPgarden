#include "driver/gpio.h"

#define LIGHTING_GPIO GPIO_NUM_32

void lightingInit() {

    gpio_reset_pin(LIGHTING_GPIO);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(LIGHTING_GPIO, GPIO_MODE_OUTPUT);
}

void lightingTurnON() {
    gpio_set_level(LIGHTING_GPIO, 1);
}

void lightingTurnOFF() {
    gpio_set_level(LIGHTING_GPIO, 0);
}