#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "tExecutor.h"


// TaskHandle_t ExecutorHandle;
static QueueHandle_t  controllerQueue = NULL;
static const uint8_t eventQueueLen = 3;

void SendExecutorEvent(const executorEvent event) { xQueueSend(controllerQueue, &event, 0); }

static void HandleEvent(const executorEvent event) {
    
    switch (event){
        case OnLight:
        //do something
        break;
        
        case OffLight:

        break;
    }
}

void ExecutorTask(void* pvParameters) {
    (void)pvParameters;

    controllerQueue = xQueueCreate(eventQueueLen, sizeof(executorEvent));

    while (1) {
        executorEvent event;
        if(xQueueReceive(controllerQueue, &event, portMAX_DELAY)){
            HandleEvent(event);
        }
    }
}
