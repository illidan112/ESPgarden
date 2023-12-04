#include "driver/gpio.h"

#define FAN_GPIO GPIO_NUM_32

void fanInit() {

    gpio_reset_pin(FAN_GPIO);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(FAN_GPIO, GPIO_MODE_OUTPUT);
}

void fanTurnON() {
    gpio_set_level(FAN_GPIO, 1);
}

void fanTurnOFF() {
    gpio_set_level(FAN_GPIO, 0);
}