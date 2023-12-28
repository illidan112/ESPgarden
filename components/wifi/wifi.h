#ifndef WIFI_H
#define WIFI_H

#include "esp_err.h"
#include <stdbool.h>

// Initialize and start the Wi-Fi in STA mode
esp_err_t wifi_sta_init();

esp_err_t wifi_sta_reset() ;

bool isWifiConnected();

#endif // WIFI_H
