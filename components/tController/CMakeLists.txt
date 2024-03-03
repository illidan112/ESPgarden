idf_component_register(SRCS "tController.c"
                    INCLUDE_DIRS "."
                    REQUIRES wifi httpServer GPIO realtime soilHumidity settings airSensor "freertos")
