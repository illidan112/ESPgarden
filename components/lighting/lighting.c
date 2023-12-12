#include "driver/gpio.h"
#include "esp_log.h"

#define LIGHTING1_GPIO GPIO_NUM_32
#define LIGHTING2_GPIO GPIO_NUM_33

static uint8_t LighingState = 0;
const static char* TAG = "LAMP";

void lightingInit() {

    gpio_reset_pin(LIGHTING1_GPIO);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(LIGHTING1_GPIO, GPIO_MODE_OUTPUT);
    LighingState = gpio_get_level(LIGHTING1_GPIO);

}

void lightingTurnON() {
    if (!(LighingState)) {
        gpio_set_level(LIGHTING1_GPIO, 1);
        LighingState = 1;
        ESP_LOGI(TAG, "ON lighting");
    }
}

void lightingTurnOFF() {
    if (LighingState) {
        gpio_set_level(LIGHTING1_GPIO, 0);
        LighingState = 0;
        ESP_LOGI(TAG, "OFF lighting");
    }

}