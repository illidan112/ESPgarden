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

static void HandleEvent(const controllerEvent event) {

    switch (event) {
    case Scan:
        //SCANNING INPUT PARAM


        /*_____TIME_CHECK______*/
        if (hoursNow() >= settings.lightTime.turnOnHour) {
            if (hoursNow() >= (settings.lightTime.turnOnHour + settings.lightTime.durationHours)) {
                // turn OFF lighting
                lightingTurnOFF();
                // SendExecutorEvent(OffLight);
            } else {
                // turn ON lighting
                lightingTurnON();
            }
        }

        /*______FAN______*/
        if (getTemp() >= settings.airTemp.MaxTemp) {
            fanTurnON(BOX_VENT);
            // SendExecutorEvent(OnFan);
            ESP_LOGI(TAG, "Temp higher than %d", settings.airTemp.MaxTemp);
        } else {
            // SendExecutorEvent(OffFan);
            fanTurnOFF(BOX_VENT);
        }
        

        /*LOGS_OF_SYSTEM*/
        char* dataTimeStr;
        dataTimeStr = getStrDateTime();

        ESP_LOGI(TAG, "%s: Temp %d°С, Humd %d%%", dataTimeStr, getTemp(), getHumidity());
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
    updateSwitchTime(22, 0, 1);

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
