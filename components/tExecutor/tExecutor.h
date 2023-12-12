#include <stdint.h>

// Available events in priority order
typedef enum {
    OnLight = 0,
    OffLight,
    OnFan,
    OffFan,

}executorEvent;

void ExecutorTask(void* pvParameters);
void SendExecutorEvent(const executorEvent event);