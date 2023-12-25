#include <stdint.h>

#ifndef TCONTROLLER_H
#define TCONTROLLER_H

// Available events in priority order
typedef enum {
    SCAN = 0,
    WIFI_LOST,

}controllerEvent;

void ControllerTask(void* pvParameters);
void SendControllerEvent(const controllerEvent event);

#endif // TCONTROLLER_H