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
#include "lighting.h"
// #include "tExecutor.h"

const static char* TAG = "CTRL";

extern SettingsData settings;

TaskHandle_t ControllerHandle;
static QueueHandle_t event_queue = NULL;
static TimerHandle_t scan_timer = NULL;
static const uint8_t eventQueueLen = 3;

void SendControllerEvent(const controllerEvent event) { xQueueSend(event_queue, &event, 0); }

void scanTmrCallback() {
    controllerEvent event = Scan;
    SendControllerEvent(event);
}

static void LightCheck(uint8_t currentHour) {
    static bool isLightON = false;

    if (currentHour >= settings.lightTime.turnOnHour && currentHour < settings.lightTime.turnOffHour) {
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

static void LampFanCheck(uint8_t currentHour) {
    static bool isFanON = false;

    if (currentHour >= settings.lightTime.turnOnHour && currentHour < (settings.lightTime.turnOffHour + 1)) {
        if (!isFanON) {
            fanTurnON(LAMP_VENT);
            isFanON = true;
        }
    } else {
        if (isFanON) {
            fanTurnOFF(LAMP_VENT);
            isFanON = false;
        }
    }
}

static void BoxFanCheck(uint8_t currentTemp) {
    static bool isFanON = false;
    
    if (currentTemp >= settings.airTemp.MaxTemp) {
        if (!isFanON) {
            fanTurnON(BOX_VENT);
            ESP_LOGI(TAG, "Temp higher than %d", settings.airTemp.MaxTemp);
            isFanON = true;
        }
    } else {
        if (isFanON) {
            fanTurnOFF(BOX_VENT);
            isFanON = false;
        }
    }
}

static void HandleEvent(const controllerEvent event) {
    switch (event) {
    case Scan:
        uint8_t currentHour = hoursNow();
        uint8_t currentTemp = getTemp();
        uint8_t currentHumidity = getHumidity();

        LightCheck(currentHour);
        BoxFanCheck(currentTemp);

        char* dataTimeStr = getStrDateTime();
        ESP_LOGI(TAG, "%s: Temp %d°C, Humd %d%%", dataTimeStr, currentTemp, currentHumidity);
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

    scan_timer = xTimerCreate("Scan Measures Tmr", pdMS_TO_TICKS(3000), pdTRUE, 0, scanTmrCallback);
    xTimerStart(scan_timer, 0);

    event_queue = xQueueCreate(eventQueueLen, sizeof(controllerEvent));

    while (1) {
        controllerEvent event;
        if (xQueueReceive(event_queue, &event, portMAX_DELAY)) {
            HandleEvent(event);
        }
    }
}
