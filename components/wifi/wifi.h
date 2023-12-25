#ifndef WIFI_H
#define WIFI_H

#include "esp_err.h"

// Initialize and start the Wi-Fi in STA mode
esp_err_t wifi_sta_init();

void wifi_sta_reset();

#endif // WIFI_H
