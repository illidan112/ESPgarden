#include <stdint.h>

#ifndef governor_H
#define governor_H

// Available events in priority order
typedef enum {
    SCAN = 0,

} governorEvent;

void GovernorTask(void* pvParameters);
void SendGovernorEvent(const governorEvent event);

#endif // governor_H