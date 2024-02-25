#include "settings.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "nvs_flash.h"
#include <stdio.h>
#include "esp_log.h"

#include "realtime.h"

const static char* TAG = "SETT";

SettingsData settings;

SemaphoreHandle_t lightTimeMutex;
SemaphoreHandle_t airTempMutex;

TaskHandle_t SettHandle;
static QueueHandle_t qSettEvent = NULL;
static TimerHandle_t tUnixSaver = NULL;
static const uint8_t eventQueueLen = 3;

esp_err_t err;

void SendSettEvent(const settEvent event) { xQueueSend(qSettEvent, &event, 0); }

// Function to initialize the settings data
void initializeSettings() {
    // Set default values
    settings.lightTime.turnOnHour = 0;
    settings.lightTime.turnOnMinute = 0;
    settings.lightTime.turnOffHour = 0;
    settings.lightTime.turnOffMinute = 0;
    settings.lightTime.durationHours = 0;
    settings.airTemp.MaxTemp = 25;
    // settings.airTemp.MinTemp = 20;

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

static void storeTime() {
    nvs_handle_t my_handle;
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

static void HandleEvent(const settEvent event) {
    switch (event) {
    case STORE:

        storeTime();

        break;
    }
}

void SettingsTask(void* pvParameters) {
    (void)pvParameters;

    qSettEvent = xQueueCreate(eventQueueLen, sizeof(settEvent));

    tUnixSaver = xTimerCreate("test Tmr", pdMS_TO_TICKS(1000), pdTRUE, 0, UnixSave_Callback);
    xTimerStart(tUnixSaver, 0);

    while (1) {
        settEvent event;
        if (xQueueReceive(qSettEvent, &event, portMAX_DELAY)) {
            HandleEvent(event);
        }
    }
}