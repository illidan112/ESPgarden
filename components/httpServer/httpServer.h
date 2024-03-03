#ifndef HTTP_SERV_H
#define HTTP_SERV_H

#include "esp_err.h"
#include <stdbool.h>

// Available events in priority order
typedef enum {
    START = 0,
    STOP,
    CONN_LOST,

} serverEvent;

void SendServerEvent(const serverEvent event);

void ServerTask(void* pvParameters);

// Start the HTTP server
esp_err_t http_server_start(void);

#endif // HTTP_SERV_H
