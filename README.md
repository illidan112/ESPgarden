# ESPgarden

## Overview
ESPGarden is based on the ESP32 microcontroller system to control grow system.

## Features:
- **Light control:** Can control light based on RTC.
- **HTTP Server:** HTTP website as interface for users. With server you can control and configure system.
- **Microclimat control:** The system has temperature and humidity sensor, which show state of your garden in http webpage.


## TODO:
- **Water irrigation:** Need to add soil sensor and water pump to automate irrigation
- **More fans:** System can be expanded more fans for better microclimat control

## Configuration:
- **HTTPD_MAX_REQ_HDR_LEN:** 1280
- **HTTPD_MAX_URI_LEN:** 1280
- **CONFIG_GPIO_CTRL_FUNC_IN_IRAM:** true
