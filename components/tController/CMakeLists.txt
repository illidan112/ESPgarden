idf_component_register(SRCS "tController.c"
                    INCLUDE_DIRS "."
                    REQUIRES realtime soilHumidity settings airSensor tExecutor "freertos")
