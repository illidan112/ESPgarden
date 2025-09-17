idf_component_register(SRCS "http_bridge.c"
                      INCLUDE_DIRS "."
                      REQUIRES event_bus http_server wifi)
