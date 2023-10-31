#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/timers.h"

// #include "soilHumidity.h"
#include "realtime.h"

#include "tController.h"

TaskHandle_t ControllerHandle;
static QueueHandle_t  event_queue = NULL;
static TimerHandle_t scan_timer = NULL;
static const uint8_t eventQueueLen = 3;

void SendControllerEvent(const controllerEvent event) { xQueueSend(event_queue, &event, 0); }

void scanTmrCallback() {
    controllerEvent event = Scan;
    SendControllerEvent(event);
}

static void HandleEvent(const controllerEvent event) {
    
    switch (event){
        case Scan:
        stringTime();
        break;
    }
}

void ControllerTask(void* pvParameters) {
    (void)pvParameters;

    scan_timer = xTimerCreate("Scan Measures Tmr", pdMS_TO_TICKS(2000), pdTRUE, 0, scanTmrCallback);

    event_queue = xQueueCreate(eventQueueLen, sizeof(controllerEvent));

    while (1) {
        controllerEvent event;
        if(xQueueReceive(event_queue, &event, portMAX_DELAY)){
            HandleEvent(event);
        }
        // Task code goes here
        vTaskDelay(pdMS_TO_TICKS(1000)); // Delay for 1000 milliseconds
    }
}
