idf_component_register(SRCS "tController.c"
                    INCLUDE_DIRS "."
                    REQUIRES lighting fan realtime soilHumidity settings airSensor "freertos")
