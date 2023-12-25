idf_component_register(SRCS "airSensor.c"
                    INCLUDE_DIRS "."
                    REQUIRES bme280 "freertos")
