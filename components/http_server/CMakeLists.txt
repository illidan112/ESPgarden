idf_component_register(SRCS "http_server.c"
                    INCLUDE_DIRS "."
                    REQUIRES wifi realtime airSensor lwip esp_wifi esp_netif esp_http_server nvs_flash settings mdns)
