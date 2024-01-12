#include "airSensor.h"
#include "bmp280.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
// #include "semphr.h"
#include "esp_log.h"
#include <string.h>

#define I2C_SDA 21
#define I2C_SCL 22

SemaphoreHandle_t sensorMutex;
static bmp280_t bme280Desc;

static uint8_t lastTemp = 0;
static uint8_t lastHumd = 0;

static const char* TAG = "AIR";

esp_err_t airSensorInit() {
    bmp280_params_t params;
    bmp280_init_default_params(&params);
    memset(&bme280Desc, 0, sizeof(bmp280_t));

    if (bmp280_init_desc(&bme280Desc, BMP280_I2C_ADDRESS_0, I2C_NUM_0, I2C_SDA, I2C_SCL) &&
        bmp280_init(&bme280Desc, &params) != ESP_OK) {

        ESP_LOGE(TAG, "BME280 init error");
        return ESP_FAIL;
    }

    bool bme280p = bme280Desc.id == BME280_CHIP_ID;
    ESP_LOGI(TAG, "Found %s\n", bme280p ? "BME280" : "BMP280");

    sensorMutex = xSemaphoreCreateMutex();

    return ESP_OK;
}

esp_err_t getAirData(int32_t* temp, uint32_t* hum) {
    uint32_t buff;
    esp_err_t status;

    if (xSemaphoreTake(sensorMutex, portMAX_DELAY)) {
        status = bmp280_read_fixed(&bme280Desc, temp, &buff, hum);
        xSemaphoreGive(sensorMutex);

        if (status != ESP_OK) {
            ESP_LOGE(TAG, "Cant read bme280");
            return ESP_FAIL;
        }

        lastHumd = *hum;
        lastTemp = *temp;
    }
    return ESP_OK;
}

uint8_t getHumidity() { return lastHumd; }

uint8_t getTemp() { return lastTemp; }
