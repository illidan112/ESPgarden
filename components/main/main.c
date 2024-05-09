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
#include "servo.h"
#include "settings.h"
#include "httpServer.h"

#define STACK_SIZE 1024
#define HIGH_PRIORITY 2
#define LOW_PRIORITY 1

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
    ESP_ERROR_CHECK(i2cdev_init());     // Initialize I2C for bme280 and ds3231

    initializeSettings();

    xTaskCreate(ControllerTask, "Plant control", STACK_SIZE * 2, NULL, LOW_PRIORITY, NULL);
    xTaskCreate(ServerTask, "Server Task", STACK_SIZE * 5, NULL, LOW_PRIORITY, NULL);
    xTaskCreate(SettingsTask, "Settings Task", STACK_SIZE * 3, NULL, HIGH_PRIORITY, NULL);
    xTaskCreate(ServoTask, "Servo Task", STACK_SIZE, NULL, LOW_PRIORITY, NULL);
}
