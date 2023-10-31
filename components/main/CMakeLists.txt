idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "."
                    REQUIRES tExecutor tController "freertos")

# target_link_libraries(${COMPONENT_LIB} PUBLIC soilHumidity) 