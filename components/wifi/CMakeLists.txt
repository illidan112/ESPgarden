idf_component_register(SRCS "wifi.c"
                    INCLUDE_DIRS "."
                    REQUIRES event_bus esp_wifi esp_event nvs_flash esp_netif)
