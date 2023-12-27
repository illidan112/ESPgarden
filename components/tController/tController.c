#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "freertos/timers.h"

// #include "soilHumidity.h"
#include "airSensor.h"
#include "realtime.h"
#include "settings.h"
// #include "tExecutor.h"
#include "tController.h"

#include "fan.h"
#include "httpServer.h"
#include "lighting.h"
#include "wifi.h"
// #include "tExecutor.h"

#define WI_FI_RECCONECT_MS 30000

const static char* TAG = "CTRL";

extern SettingsData settings;

TaskHandle_t ControllerHandle;
static QueueHandle_t event_queue = NULL;
static TimerHandle_t scan_timer = NULL;
static TimerHandle_t serv_rest = NULL;
static const uint8_t eventQueueLen = 3;

void SendControllerEvent(const controllerEvent event) { xQueueSend(event_queue, &event, 0); }

void scanTmrCallback() {
    controllerEvent event = SCAN;
    SendControllerEvent(event);
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

static void LightCheck(uint8_t currentHour) {
    static bool isLightON = false;
    uint8_t onHour = 0;
    uint8_t offHour = 0;

    getLightTime(&onHour, &offHour);

    if (currentHour >= onHour && currentHour < offHour) {
        if (!isLightON) {
            lightingTurnON();
            isLightON = true;
        }
    } else {
        if (isLightON) {
            lightingTurnOFF();
            isLightON = false;
        }
    }
}

// static void LampFanCheck(uint8_t currentHour) {
//     static bool isFanON = false;

//     if (currentHour >= settings.lightTime.turnOnHour && currentHour < (settings.lightTime.turnOffHour + 1)) {
//         if (!isFanON) {
//             fanTurnON(LAMP_VENT);
//             isFanON = true;
//         }
//     } else {
//         if (isFanON) {
//             fanTurnOFF(LAMP_VENT);
//             isFanON = false;
//         }
//     }
// }

static void BoxFanCheck(uint8_t currentTemp) {
    static bool isFanON = false;
    uint8_t maxT = 0;
    uint8_t minT = 0;

    getAirTemp(&maxT, &minT);

    if (currentTemp >= maxT) {
        if (!isFanON) {
            fanTurnON(BOX_VENT);
            ESP_LOGI(TAG, "Temp higher than %d", maxT);
            isFanON = true;
        }
    } else {
        if (isFanON) {
            fanTurnOFF(BOX_VENT);
            isFanON = false;
        }
    }
}

static void ServerRestart() {
    wifi_sta_reset();
    if (!isHttpServerActive()) {
        ESP_ERROR_CHECK(http_server_start());
    }
}

static void HandleEvent(const controllerEvent event) {
    switch (event) {
    case SCAN:
        uint8_t currentHour = hoursNow();
        uint8_t currentTemp = getTemp();
        uint8_t currentHumidity = getHumidity();

        LightCheck(currentHour);
        BoxFanCheck(currentTemp);

        char* dataTimeStr = getStrDateTime();
        ESP_LOGI(TAG, "%s: Temp %d°C, Humd %d%%", dataTimeStr, currentTemp, currentHumidity);
        break;

    case SERVER_RESTART:
        // after expiring wifi_sta_reset() will be call
        
            if (xTimerIsTimerActive(serv_rest) == pdFALSE) {
                xTimerStart(serv_rest, 0);
            }

        ESP_LOGI(TAG, "SERVER_RESTART Handled");
        break;

    }
}

void ControllerTask(void* pvParameters) {
    (void)pvParameters;

    timeInit();
    airSensorInit();
    lightingInit();
    fanInit();

    /*TODO: Initialization of all setting
    should be in another place*/
    updateSwitchTime(22, 23);

    scan_timer = xTimerCreate("Scan Measures Tmr", pdMS_TO_TICKS(5000), pdTRUE, 0, scanTmrCallback);
    serv_rest = xTimerCreate("WI-FI connection delay", pdMS_TO_TICKS(WI_FI_RECCONECT_MS), pdFALSE, (void*)0, ServerRestart);
    xTimerStart(scan_timer, 0);

    event_queue = xQueueCreate(eventQueueLen, sizeof(controllerEvent));

    esp_err_t status = wifi_sta_init();
    if (status == ESP_ERR_TIMEOUT) {
        // controllerEvent event_rest = SERVER_RESTART;
        // SendControllerEvent(event_rest);
        ESP_LOGW(TAG, "WIFI_ERR_TIMEOUT");
        if (xTimerIsTimerActive(serv_rest) == pdFALSE) {
            xTimerStart(serv_rest, 0);
        }
    } else if (status == ESP_OK) {
        ESP_ERROR_CHECK(http_server_start());
    } else {
        ESP_LOGE(TAG, "WIFI Init ERROR");
    }

    while (1) {
        controllerEvent event;
        if (xQueueReceive(event_queue, &event, portMAX_DELAY)) {
            HandleEvent(event);
        }
    }
}
