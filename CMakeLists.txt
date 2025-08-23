cmake_minimum_required(VERSION 3.16)

set(EXTRA_COMPONENT_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/tools/esp-idf-lib/components)

# Limit build to only required components and their dependencies
set(COMPONENTS
  # Project components
  main
  httpServer
  tController
  wifi
  GPIO
  realtime
  soilHumidity
  settings
  airSensor

  # External components from esp-idf-lib (via EXTRA_COMPONENT_DIRS)
  bmp280
  ds3231
  i2cdev

  # ESP-IDF base components needed by our components
  freertos
  driver
  nvs_flash
  esp_event
  esp_netif
  lwip
  esp_wifi
  esp_http_server
  mdns
)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(ESPGARDEN)

# add_subdirectory(components)