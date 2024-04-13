#ifndef WIFI_H
#define WIFI_H

#include "esp_err.h"
#include <stdbool.h>

// Only initialize the Wi-Fi in STA mode
esp_err_t wifi_sta_init();

// Scanning and connecting to the WI-FI point
esp_err_t wifi_reconnect(void);


#endif // WIFI_H
