idf_component_register(SRCS "wifi.c"
                    INCLUDE_DIRS "."
                    REQUIRES tController esp_wifi esp_event nvs_flash esp_netif)
