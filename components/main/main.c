// #include "driver/gpio.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_log.h"
#include "esp_event.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>
#include <stdlib.h>
// #include <time.h>
// #include <sys/time.h>

#include "tController.h"
// #include "tExecutor.h"
#include "settings.h"

#define STACK_SIZE 4096
#define HIGH_PRIORITY 1

#define BLINK_GPIO GPIO_NUM_32

// const static char* TAG = "MAIN";

void app_main(void) { 

    //Initialize NVS
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    initializeSettings();

    xTaskCreate(ControllerTask, "Controller Task", STACK_SIZE, NULL, HIGH_PRIORITY, NULL); 
    // xTaskCreate(ExecutorTask, "Executor Task", STACK_SIZE, NULL, HIGH_PRIORITY, NULL);
}
