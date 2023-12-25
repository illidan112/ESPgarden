#include "airSensor.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "bme280.h"
// #include "semphr.h"
#include "esp_log.h"

SemaphoreHandle_t sensorMutex;
bme280_data_t bme280_data = {};

static const char *TAG = "AIR";

void airSensorInit() {
    bme280_init();
    sensorMutex = xSemaphoreCreateMutex();
}

uint8_t getHumidity() {
    uint8_t humidity = 0;

    if (xSemaphoreTake(sensorMutex, portMAX_DELAY)) {
        bme280_get_data(&bme280_data);
        if (bme280_data.humidity >= 0 && bme280_data.humidity <= 100) {
            humidity = bme280_data.humidity;
        } else {
            ESP_LOGE(TAG,"0 =< getHumidity <= 100");
            return 0;
        }
        xSemaphoreGive(sensorMutex);
    }

    return humidity;
}

uint8_t getTemp() {
    uint8_t temperature = 0;

    if(xSemaphoreTake(sensorMutex, portMAX_DELAY)) {
        bme280_get_data(&bme280_data);
        if(bme280_data.temperature >= -40 && bme280_data.temperature <= 85) {
            temperature = bme280_data.temperature;
        } else {
            ESP_LOGE(TAG,"-40 =< getHumidity <= 85");
            return 0;
        }
        xSemaphoreGive(sensorMutex);
    }

    return temperature;
}
