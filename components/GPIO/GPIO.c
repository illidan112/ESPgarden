#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "httpServer.h"

#define ESP_INTR_FLAG_DEFAULT 0

#define LIGHTING1 GPIO_NUM_32
#define LIGHTING2 GPIO_NUM_33
#define LIGHTING3 GPIO_NUM_25
#define LIGHTING4 GPIO_NUM_26
#define FAN1 GPIO_NUM_14
#define BUTTON1 GPIO_NUM_23

static bool LighingState = false;
const static char* TAG = "GPIO";

static void IRAM_ATTR gpio_isr_handler(void* arg) {
    if (!gpio_get_level(BUTTON1)) {
        gpio_intr_disable(BUTTON1);
        serverEvent event = RECONNECT;
        SendServerEventISR(event);
        gpio_intr_enable(BUTTON1);
    }
}

void EnableButton() {
    gpio_intr_enable(BUTTON1);
}

void lightingInit() {

    gpio_reset_pin(LIGHTING1);
    gpio_reset_pin(LIGHTING2);
    gpio_reset_pin(LIGHTING3);
    gpio_reset_pin(LIGHTING4);
    gpio_set_direction(LIGHTING1, GPIO_MODE_OUTPUT);
    gpio_set_direction(LIGHTING2, GPIO_MODE_OUTPUT);
    gpio_set_direction(LIGHTING3, GPIO_MODE_OUTPUT);
    gpio_set_direction(LIGHTING4, GPIO_MODE_OUTPUT);
    // LighingState = gpio_get_level(LIGHTING1);

    // BUTTON FOR WIFI
    gpio_reset_pin(BUTTON1);
    gpio_set_intr_type(BUTTON1, GPIO_INTR_POSEDGE);
    gpio_set_direction(BUTTON1, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON1, GPIO_PULLUP_ONLY);
    // install gpio isr service
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    // hook isr handler for specific gpio pin
    gpio_isr_handler_add(BUTTON1, gpio_isr_handler, (void*)BUTTON1);
}

void lightingTurnON() {
    if (!(LighingState)) {
        gpio_set_level(LIGHTING1, 1);
        gpio_set_level(LIGHTING2, 1);
        vTaskDelay(pdMS_TO_TICKS(500));
        gpio_set_level(LIGHTING3, 1);
        gpio_set_level(LIGHTING4, 1);
        LighingState = true;
        ESP_LOGI(TAG, "ON LIGHT");
    }
}

void lightingTurnOFF() {
    if (LighingState) {
        gpio_set_level(LIGHTING1, 0);
        gpio_set_level(LIGHTING2, 0);
        vTaskDelay(pdMS_TO_TICKS(500));
        gpio_set_level(LIGHTING3, 0);
        gpio_set_level(LIGHTING4, 0);
        LighingState = false;
        ESP_LOGI(TAG, "OFF LIGHT");
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