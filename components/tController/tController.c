#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "freertos/timers.h"

#include "servo.h"
#include "GPIO.h"
#include "airSensor.h"
#include "httpServer.h"
#include "realtime.h"
#include "settings.h"
#include "tController.h"
#include "wifi.h"

#define WI_FI_RECCONECT_MS 10000

const static char* TAG = "CTRL";

extern SettingsData settings;

TaskHandle_t ControllerHandle;
static QueueHandle_t event_queue = NULL;
static TimerHandle_t scan_timer = NULL;
static const uint8_t eventQueueLen = 3;

void SendControllerEvent(const controllerEvent event) { xQueueSend(event_queue, &event, 0); }

void scanTmrCallback() {
    controllerEvent event = SCAN;
    SendControllerEvent(event);
}

static void LightCheck(uint8_t currentHour) {
    static bool isLightON = false;
    uint8_t onHour = 0;
    uint8_t offHour = 0;

    getLightTime(&onHour, &offHour);

    // Logic to determine whether to turn the light on or off
    if (onHour <= offHour) {
        //  Scenario when both turning on and off happen within the same day
        if (isLightON) {
            if (currentHour >= offHour || currentHour < onHour) {
                lightingTurnOFF();
                suspendRotation();
                isLightON = false;
            }
        } else {
            if (currentHour >= onHour && currentHour < offHour) {
                lightingTurnON();
                resumeRotation();
                isLightON = true;
            }
        }
    } else {
        // Scenario when turning on or off happens across midnight
        if (isLightON) {
            if (currentHour >= offHour && currentHour < onHour) {
                lightingTurnOFF();
                suspendRotation();
                isLightON = false;
            }
        } else {
            if (currentHour >= onHour || currentHour < offHour) {
                lightingTurnON();
                resumeRotation();
                isLightON = true;
            }
        }
    }
}

static void BoxFanCheck(uint8_t currentTemp) {
    static bool isFanON = false;
    uint8_t maxT = 0;
    uint8_t minT = 0;

    getAirTemp(&maxT, &minT);

    if (currentTemp >= maxT) {
        if (!isFanON) {
            fanTurnON();
            ESP_LOGI(TAG, "Temp higher than %d", maxT);
            isFanON = true;
        }
    } else {
        if (isFanON) {
            fanTurnOFF();
            isFanON = false;
        }
    }
}

static void HandleEvent(const controllerEvent event) {
    switch (event) {
    case SCAN:
        float currentTemp;
        float currentHumidity;
        uint8_t currentHour = hoursNow();

        if (getAirData(&currentTemp, &currentHumidity) == ESP_OK) {
            BoxFanCheck((uint8_t)currentTemp);
        } else {
            ESP_LOGE(TAG, "Cant get air data");
        }

        LightCheck(currentHour);

        char* dataTimeStr = getStrDateTime();
        ESP_LOGI(TAG, "%s: Temp %.2f°C, Humd %.2f%%", dataTimeStr, currentTemp, currentHumidity);
        break;
    }
}

void ControllerTask(void* pvParameters) {
    (void)pvParameters;

    ESP_ERROR_CHECK(airSensorInit());
    ESP_ERROR_CHECK(timeInit());
    lightingInit();
    fanInit();

    scan_timer = xTimerCreate("Scan Measures Tmr", pdMS_TO_TICKS(5000), pdTRUE, 0, scanTmrCallback);
    xTimerStart(scan_timer, 0);

    event_queue = xQueueCreate(eventQueueLen, sizeof(controllerEvent));

    while (1) {
        controllerEvent event;
        if (xQueueReceive(event_queue, &event, portMAX_DELAY)) {
            HandleEvent(event);
        }
    }
}

// void startOneShotTimer(void (*callbackFunction)(TimerHandle_t)) {

//     // Создание таймера
//     xOneShotTimer = xTimerCreate("Timer", pdMS_TO_TICKS(1000), pdFALSE, (void *) 0, (TimerCallbackFunction_t)
//     callbackFunction);

//     if(xOneShotTimer == NULL) {
//         // Обработка ошибки создания таймера
//     } else {
//         // Запуск таймера. Не перезапускается после срабатывания
//         if(xTimerStart(xOneShotTimer, 0) != pdPASS) {
//             // Обработка ошибки запуска таймера
//         }
//     }
// }

// void stopOneShotTimer(){
//     xTimerDelete(xOneShotTimer, 0);
// }