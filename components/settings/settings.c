#include "settings.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "nvs_flash.h"
#include <stdio.h>

#include "realtime.h"

const static char* TAG = "SETT";

SettingsData settings;

SemaphoreHandle_t lightTimeMutex;
SemaphoreHandle_t airTempMutex;

TaskHandle_t SettHandle;
static QueueHandle_t qSettEvent = NULL;
static TimerHandle_t tUnixSaver = NULL;
static const uint8_t eventQueueLen = 3;

void SendSettEvent(const settEvent event) { xQueueSend(qSettEvent, &event, 0); }

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
        settings.lightTime.turnOnHour = 1;
        settings.lightTime.turnOnMinute = 0;
        settings.lightTime.turnOffHour = 21;
        settings.lightTime.turnOffMinute = 0;
        settings.airTemp.MaxTemp = 26;
        settings.airTemp.MinTemp = 0;
    } else {
        ESP_LOGW(TAG, "Got settings from NVS");
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

void UnixSave_Callback() {
    settEvent event = STORE;
    SendSettEvent(event);
}

static void storeDS3231Time() {
    nvs_handle_t my_handle;
    esp_err_t err;
    err = nvs_open("storage", NVS_READWRITE, &my_handle); // Opening Non-Volatile Storage (NVS)
    if (err == ESP_OK) {
        int32_t unixTime = UnixTime();

        if (nvs_set_i32(my_handle, "unix", unixTime) != ESP_OK) { // Write
            ESP_LOGE(TAG, "NVS write unix time ERROR.");
        }

        // Commit written value.
        // After setting any values, nvs_commit() must be called to ensure changes are written
        // to flash storage. Implementations may write to storage at other times,
        // but this is not guaranteed
        if (nvs_commit(my_handle) != ESP_OK) {
            ESP_LOGE(TAG, "NVS commit unix time ERROR.");
        }

        nvs_close(my_handle); // Closing NVS

    } else {
        ESP_LOGE(TAG, "NVS open ERROR.");
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

static void HandleEvent(const settEvent event) {
    switch (event) {
    case STORE:
        ESP_LOGI(TAG, "Store all settings");
        // storeTime();
        storeAllSettgs();

        break;
    }
}

void SettingsTask(void* pvParameters) {
    (void)pvParameters;

    qSettEvent = xQueueCreate(eventQueueLen, sizeof(settEvent));

    tUnixSaver = xTimerCreate("test Tmr", pdMS_TO_TICKS(1000), pdTRUE, 0, UnixSave_Callback);
    // xTimerStart(tUnixSaver, 0);

    while (1) {
        settEvent event;
        if (xQueueReceive(qSettEvent, &event, portMAX_DELAY)) {
            HandleEvent(event);
        }
    }
}