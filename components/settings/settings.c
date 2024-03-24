#include "settings.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "nvs_flash.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "realtime.h"

const static char* TAG = "SETT";

SettingsData settings;

SemaphoreHandle_t lightTimeMutex;
SemaphoreHandle_t airTempMutex;

TaskHandle_t SettHandle;
static QueueHandle_t qSettEvent = NULL;
static const uint8_t eventQueueLen = 3;
static char globalBuf[RESPONSE_BUF_SIZE];


void SendSettEvent(const settEvent event) { xQueueSend(qSettEvent, &event, 0U); }

void sendRawResponse(char* buf) {
    strcpy(globalBuf, buf);
}

static esp_err_t getAllSettgs() {
    nvs_handle_t my_handle;
    esp_err_t err;
    err = nvs_open("storage", NVS_READONLY, &my_handle); // Opening Non-Volatile Storage (NVS)
    if (err == ESP_OK) {

        // TIME
        err = nvs_get_u8(my_handle, "turnOnHour", &settings.lightTime.turnOnHour);
        if (err == ESP_ERR_NVS_NOT_FOUND) {
            return ESP_ERR_NVS_NOT_FOUND;
        } else if (err != ESP_OK) {
            ESP_LOGE(TAG, "NVS ERROR.");
        }

        err = nvs_get_u8(my_handle, "turnOffHour", &settings.lightTime.turnOffHour);
        if (err == ESP_ERR_NVS_NOT_FOUND) {
            return ESP_ERR_NVS_NOT_FOUND;
        } else if (err != ESP_OK) {
            ESP_LOGE(TAG, "NVS ERROR.");
        }

        // TEMP
        err = nvs_get_u8(my_handle, "MinTemp", &settings.airTemp.MinTemp);
        if (err == ESP_ERR_NVS_NOT_FOUND) {
            return ESP_ERR_NVS_NOT_FOUND;
        } else if (err != ESP_OK) {
            ESP_LOGE(TAG, "NVS ERROR.");
        }

        err = nvs_get_u8(my_handle, "MaxTemp", &settings.airTemp.MaxTemp);
        if (err == ESP_ERR_NVS_NOT_FOUND) {
            return ESP_ERR_NVS_NOT_FOUND;
        } else if (err != ESP_OK) {
            ESP_LOGE(TAG, "NVS ERROR.");
        }

        nvs_close(my_handle); // Closing NVS

    } else {
        ESP_LOGE(TAG, "NVS open ERROR.");
    }
    return ESP_OK;
}

// Function to initialize the settings data
void initializeSettings() {
    if (getAllSettgs() == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGW(TAG, "First init, get default settings");
        // Set default values
        settings.lightTime.turnOnHour = 6;
        settings.lightTime.turnOnMinute = 0;
        settings.lightTime.turnOffHour = 0;
        settings.lightTime.turnOffMinute = 0;
        settings.airTemp.MaxTemp = 25;
        settings.airTemp.MinTemp = 18;
    } else {
        ESP_LOGW(TAG, "Got settings from NVS");
        ESP_LOGI(TAG, "turnOnHour:%d, turnOffHour:%d, MaxTemp:%d", settings.lightTime.turnOnHour,
                 settings.lightTime.turnOffHour, settings.airTemp.MaxTemp);
    }

    lightTimeMutex = xSemaphoreCreateMutex();
    airTempMutex = xSemaphoreCreateMutex();
}

// Update light switch time
void updateSwitchTime(uint8_t ONhour, uint8_t OFFhour) {
    updateTurnONTime(ONhour);
    updateTurnOFFTime(OFFhour);
}

void updateTurnONTime(uint8_t ONhour) {
    if (xSemaphoreTake(lightTimeMutex, portMAX_DELAY)) {
        settings.lightTime.turnOnHour = (uint8_t)ONhour;

        xSemaphoreGive(lightTimeMutex);
    }
}

void updateTurnOFFTime(uint8_t OFFhour) {
    if (xSemaphoreTake(lightTimeMutex, portMAX_DELAY)) {
        settings.lightTime.turnOffHour = (uint8_t)OFFhour;

        xSemaphoreGive(lightTimeMutex);
    }
}

void updateMinAirTemp(int min) {
    if (xSemaphoreTake(airTempMutex, portMAX_DELAY)) {
        settings.airTemp.MinTemp = (uint8_t)min;

        xSemaphoreGive(airTempMutex);
    }
}

void updateMaxAirTemp(int max) {
    if (xSemaphoreTake(airTempMutex, portMAX_DELAY)) {
        settings.airTemp.MaxTemp = (uint8_t)max;

        xSemaphoreGive(airTempMutex);
    }
}

void getLightTime(uint8_t* turnOnHour, uint8_t* turnOffHour) {

    if (xSemaphoreTake(lightTimeMutex, portMAX_DELAY)) {
        *turnOnHour = settings.lightTime.turnOnHour;
        *turnOffHour = settings.lightTime.turnOffHour;

        xSemaphoreGive(lightTimeMutex);
    }
}

void getAirTemp(uint8_t* maxTemp, uint8_t* minTemp) {
    if (xSemaphoreTake(airTempMutex, portMAX_DELAY)) {
        *maxTemp = settings.airTemp.MaxTemp;
        *minTemp = settings.airTemp.MinTemp;

        xSemaphoreGive(airTempMutex);
    }
}

static void storeAllSettgs() {
    nvs_handle_t my_handle;
    esp_err_t err;
    err = nvs_open("storage", NVS_READWRITE, &my_handle); // Opening Non-Volatile Storage (NVS)
    if (err == ESP_OK) {

        // TIME
        if (nvs_set_u8(my_handle, "turnOnHour", settings.lightTime.turnOnHour) != ESP_OK) {
            ESP_LOGE(TAG, "NVS write turnOnHour ERROR.");
        }

        if (nvs_set_u8(my_handle, "turnOffHour", settings.lightTime.turnOffHour) != ESP_OK) {
            ESP_LOGE(TAG, "NVS write turnOffHour ERROR.");
        }

        // TEMP
        if (nvs_set_u8(my_handle, "MinTemp", settings.airTemp.MinTemp) != ESP_OK) {
            ESP_LOGE(TAG, "NVS write MinTemp ERROR.");
        }

        if (nvs_set_u8(my_handle, "MaxTemp", settings.airTemp.MaxTemp) != ESP_OK) {
            ESP_LOGE(TAG, "NVS write MaxTemp ERROR.");
        }

        // Commit written value.
        // After setting any values, nvs_commit() must be called to ensure changes are written
        // to flash storage. Implementations may write to storage at other times,
        // but this is not guaranteed
        if (nvs_commit(my_handle) != ESP_OK) {
            ESP_LOGE(TAG, "NVS commit settings ERROR.");
        }

        nvs_close(my_handle); // Closing NVS

    } else {
        ESP_LOGE(TAG, "NVS open ERROR.");
    }
}

// example of "data": "lightOff"
int extractValue(char* data, char* tag) {
    char* startPos = strstr(data, tag);
    if (startPos == NULL) {
        ESP_LOGE(TAG, "Tag '%s' not found.", tag);
        return -2;
    }

    startPos += strlen(tag) + 1; // Moving pointer past the tag and the equal sign.
    int value = 0;

    // Находим позицию символа '&' начиная с текущей позиции startPos
    char* endPos = strchr(startPos, '&');
    if (endPos == NULL) {
        ESP_LOGE(TAG, "Invalid format.");
        return -2; // Неверный формат строки
    }

    // Создаем временную строку для хранения числа
    int len = endPos - startPos;
    if (len <= 0) {
        ESP_LOGI(TAG, "Tag is empty. Skipping");
        return -1;
    }
    char temp[16];

    strncpy(temp, startPos, len);
    temp[len] = '\0'; // Добавляем нулевой символ в конец строки

    // Конвертируем строку в число
    value = atoi(temp);
    if (value == 0 && temp[0] != '0') {
        ESP_LOGE(TAG, "Can't convert string to value.");
        return -2;
    }

    return value;
}

static void HandleEvent(const settEvent event) {
    char response[RESPONSE_BUF_SIZE];
    strcpy(response, globalBuf);
    memset(globalBuf, 0, RESPONSE_BUF_SIZE);

    switch (event) {
    case UPDATE_LIGHT_TIME:
        ESP_LOGI(TAG, "LIGHT_TIME update start");
        int bufLight = extractValue(response, "lightOff");
        if (bufLight >= 0) {
            updateTurnOFFTime(bufLight);
            ESP_LOGI(TAG, "lightOFF updated");
        }

        bufLight = extractValue(response, "lightOn");
        if (bufLight >= 0) {
            updateTurnONTime(bufLight);
            ESP_LOGI(TAG, "lightON updated");
        }

        storeAllSettgs();
        ESP_LOGI(TAG, "LIGHT_TIME update completed");
        break;

    case UPDATE_TEMP:
        ESP_LOGI(TAG, "TEMPrature update start");
        // MINIMUM TEMPERATURE
        int bufTemp = extractValue(response, "mintmp");
        if (bufTemp >= 0) {
            updateMinAirTemp(bufTemp);
        }
        // MAXIMUM TEMPERATURE
        bufTemp = extractValue(response, "maxtmp");
        if (bufTemp >= 0) {
            updateMaxAirTemp(bufTemp);
        }

        storeAllSettgs();
        ESP_LOGI(TAG, "TEMPrature update completed");
        break;

    case UPDATE_RTC:
        // char response_test[] = "time=05%3A00&date=2024-03-24&";
        struct tm tm_info;
        memset(&tm_info, 0, sizeof(struct tm));

        // Convert "%3A" in ":"
        char* encoded_colon = strstr(response, "%3A");
        if (encoded_colon) {
            *encoded_colon = ':'; // change % in :
            memmove(encoded_colon + 1, encoded_colon + 3,
                    strlen(encoded_colon) - 2); // Move the remaining part
        }

        // Template for strptime
        char* format = "time=%H:%M&date=%Y-%m-%d&";

        strptime(response, format, &tm_info);

        update_rtc(&tm_info);
        break;
    }
}

void SettingsTask(void* pvParameters) {
    (void)pvParameters;

    qSettEvent = xQueueCreate(eventQueueLen, sizeof(settEvent));

    while (1) {
        settEvent eveData;
        if (xQueueReceive(qSettEvent, &eveData, portMAX_DELAY)) {
            HandleEvent(eveData);
        }
    }
}