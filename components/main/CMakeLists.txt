idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "."
                    REQUIRES http_server http_bridge http_server governor settings nvs_flash esp_netif "freertos")

# target_link_libraries(${COMPONENT_LIB} PUBLIC soilHumidity) 