idf_component_register(SRCS "realtime.c"
                    INCLUDE_DIRS "."
                    REQUIRES ds3231 "freertos")