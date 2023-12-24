idf_component_register(SRCS "tController.c"
                    INCLUDE_DIRS "."
                    REQUIRES wifi httpServer lighting fan realtime soilHumidity settings airSensor "freertos")
