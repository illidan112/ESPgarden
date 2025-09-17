#ifndef HTTP_SERV_H
#define HTTP_SERV_H

#include "esp_err.h"
#include <stdbool.h>

// Available events in priority order
typedef enum {
    RECONNECT = 0,

} serverEvent;

void SendServerEvent(const serverEvent event);
void SendServerEventISR(const serverEvent event);

// FreeRTOS task 
void ServerTask(void* pvParameters);

// Start the HTTP server
esp_err_t http_server_start(void);

// Return state of http server
bool isWebServerRunning();

// Set state of http server
void setWebServerState(bool flag);

#endif // HTTP_SERV_H
