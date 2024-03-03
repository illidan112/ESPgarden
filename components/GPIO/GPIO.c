#include "driver/gpio.h"
#include "esp_log.h"
#include "httpServer.h"
#include "freertos/FreeRTOS.h"

#define ESP_INTR_FLAG_DEFAULT 0

#define LIGHTING1   GPIO_NUM_5
#define LIGHTING2   GPIO_NUM_33
#define BUTTON1     GPIO_NUM_32
#define FAN1        GPIO_NUM_25

static uint8_t LighingState = 0;
const static char* TAG = "GPIO";

static void IRAM_ATTR gpio_isr_handler(void* arg)
{
    serverEvent event = START;
    SendServerEvent(event);
}

void lightingInit() {

    gpio_reset_pin(LIGHTING1);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(LIGHTING1, GPIO_MODE_OUTPUT);
    LighingState = gpio_get_level(LIGHTING1);

    //BUTTON FOR WIFI
    gpio_reset_pin(BUTTON1);
    gpio_set_intr_type(BUTTON1, GPIO_INTR_POSEDGE);
    gpio_set_direction(BUTTON1, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON1, GPIO_PULLUP_ONLY);
        //install gpio isr service
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    //hook isr handler for specific gpio pin
    gpio_isr_handler_add(BUTTON1, gpio_isr_handler, (void*) BUTTON1);

}

void lightingTurnON() {
    if (!(LighingState)) {
        gpio_set_level(LIGHTING1, 1);
        LighingState = 1;
        ESP_LOGI(TAG, "ON lighting");
    }
}

void lightingTurnOFF() {
    if (LighingState) {
        gpio_set_level(LIGHTING1, 0);
        LighingState = 0;
        ESP_LOGI(TAG, "OFF lighting");
    }
}

void fanInit() {

    gpio_reset_pin(FAN1);
    // gpio_reset_pin(LAMP_VENT);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(FAN1, GPIO_MODE_OUTPUT);
    // gpio_set_direction(LAMP_VENT, GPIO_MODE_OUTPUT);
}

void fanTurnON() {
    gpio_set_level(FAN1, 1);
    ESP_LOGI(TAG, "Fan ON");
}

void fanTurnOFF() {
    gpio_set_level(FAN1, 0);
    ESP_LOGI(TAG, "Fan OFF");
}