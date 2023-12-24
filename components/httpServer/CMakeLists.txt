idf_component_register(SRCS "httpServer.c"
                    INCLUDE_DIRS "."
                    REQUIRES lwip esp_wifi esp_netif esp_http_server esp_http_server nvs_flash)
