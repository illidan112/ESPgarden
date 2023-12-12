#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include <stdbool.h>

#include "fan.h"
#include "lighting.h"
#include "tExecutor.h"

const static char* TAG = "EXE";

// TaskHandle_t ExecutorHandle;
static QueueHandle_t controllerQueue = NULL;
static const uint8_t eventQueueLen = 4;

static bool lightingState = false;
static bool FanState = false;

void SendExecutorEvent(const executorEvent event) { xQueueSend(controllerQueue, &event, 0); }

static void HandleEvent(const executorEvent event) {

    switch (event) {
    case OnLight:
        if (!(lightingState)) {
            lightingTurnON();
            lightingState = true;
            ESP_LOGI(TAG, "ON lighting");
        }
        break;

    case OffLight:
        if (lightingState) {
            lightingTurnOFF();
            lightingState = false;
            ESP_LOGI(TAG, "OFF lighting");
        }
        break;

    case OnFan:
        if (!(FanState)) {
            fanTurnON();
            FanState = true;
            ESP_LOGI(TAG, "ON FAN");
        }
        break;

    case OffFan:
        if (FanState) {
            fanTurnOFF();
            FanState = false;
            ESP_LOGI(TAG, "OFF FAN");
        }
        break;
    }
}

void ExecutorTask(void* pvParameters) {
    (void)pvParameters;

    lightingInit();

    controllerQueue = xQueueCreate(eventQueueLen, sizeof(executorEvent));

    while (1) {
        executorEvent event;
        if (xQueueReceive(controllerQueue, &event, portMAX_DELAY)) {
            HandleEvent(event);
        }
    }
}
