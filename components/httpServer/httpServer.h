#ifndef HTTP_SERV_H
#define HTTP_SERV_H

#include "esp_err.h"
#include <stdbool.h>

// Start the HTTP server
esp_err_t http_server_start(void);

bool isHttpServerActive();

// Stop the HTTP server
// esp_err_t http_server_stop(void);

#endif // HTTP_SERV_H