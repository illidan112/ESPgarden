idf_component_register(SRCS "governor.c"
                    INCLUDE_DIRS "."
                    REQUIRES event_bus GPIO realtime soilHumidity settings airSensor "freertos")
