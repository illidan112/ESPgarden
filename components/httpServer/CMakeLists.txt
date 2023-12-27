idf_component_register(SRCS "httpServer.c"
                    INCLUDE_DIRS "."
                    REQUIRES realtime airSensor lwip esp_wifi esp_netif esp_http_server nvs_flash settings)
