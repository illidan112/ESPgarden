#include <stdint.h>

#ifndef TCONTROLLER_H
#define TCONTROLLER_H

// Available events in priority order
typedef enum {
    SCAN = 0,
    SERVER_RESTART,

}controllerEvent;

// typedef struct {
//     controllerEvent eventType;
//     union {
//         struct {
//             int settingData;
//             // other variables
//         } settData;
//         // other structures
//     } eventData;
// } ControllerEventInfo;


void ControllerTask(void* pvParameters);
void SendControllerEvent(const controllerEvent event);

#endif // TCONTROLLER_H