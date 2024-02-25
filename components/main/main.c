// #include "driver/gpio.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include <stdio.h>
#include <stdlib.h>
// #include <time.h>
// #include <sys/time.h>
#include "i2cdev.h"

#include "tController.h"
// #include "tExecutor.h"
#include "settings.h"

#define STACK_SIZE 4096
#define HIGH_PRIORITY 1
#define LOW_PRIORITY 0

#define BLINK_GPIO GPIO_NUM_32

const static char* TAG = "MAIN";

void app_main(void) {

    // Initialize NVS
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_LOGE(TAG, "NVS Init ERROR. NVS will be erase");
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK( err );


    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    // Initialize I2C for bme280 and ds3231
    ESP_ERROR_CHECK(i2cdev_init());

    initializeSettings();

    xTaskCreate(ControllerTask, "Controller Task", STACK_SIZE, NULL, LOW_PRIORITY, NULL);
    xTaskCreate(SettingsTask, "Settings Task", STACK_SIZE, NULL, HIGH_PRIORITY, NULL);
    // xTaskCreate(ExecutorTask, "Executor Task", STACK_SIZE, NULL, HIGH_PRIORITY, NULL);
}
