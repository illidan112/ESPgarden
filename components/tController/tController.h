#include <stdint.h>

// Available events in priority order
typedef enum {
    Scan = 0,

}controllerEvent;

void ControllerTask(void* pvParameters);