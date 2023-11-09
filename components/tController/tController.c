#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "esp_log.h"

// #include "soilHumidity.h"
#include "bme280.h"
#include "realtime.h"
#include "settings.h"
#include "tExecutor.h"

#include "tController.h"

const static char* TAG = "CTRL";

extern SettingsData settings;

TaskHandle_t ControllerHandle;
static QueueHandle_t event_queue = NULL;
static TimerHandle_t scan_timer = NULL;
static const uint8_t eventQueueLen = 3;

bme280_data_t bme280_data = {};

void SendControllerEvent(const controllerEvent event) { xQueueSend(event_queue, &event, 0); }

void scanTmrCallback() {
    controllerEvent event = Scan;
    SendControllerEvent(event);
}

static void HandleEvent(const controllerEvent event) {

    switch (event) {
    case Scan:
        /*_____LIGHT______*/
        // printf("hoursNow(): %d\n", hoursNow());
        // printf("address settings: %d\n", (int)&settings);
        if (hoursNow() >= settings.lightTime.turnOnHour) {
            // printf("turnOnHour: %d\n", settings.lightTime.turnOnHour);
            // printf("durationHours: %d\n", settings.lightTime.durationHours);
            if (hoursNow() >= (settings.lightTime.turnOnHour + settings.lightTime.durationHours)) {
                // turn OFF lighting
                SendExecutorEvent(OffLight);
            } else {
                // turn ON lighting
                SendExecutorEvent(OnLight);
            }
        }

        bme280_get_data(&bme280_data);
        ESP_LOGI(TAG, "temperature = %d\n", bme280_data.temperature);
        ESP_LOGI(TAG, "humidity = %d\n", bme280_data.humidity);

        stringDateTime();
        break;
    }
}

void ControllerTask(void* pvParameters) {
    (void)pvParameters;

    timeInit();
    bme280_init();

    /*TODO: Initialization of all setting
    should be in another place*/
    updateSwitchTime(20, 0, 1);

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
