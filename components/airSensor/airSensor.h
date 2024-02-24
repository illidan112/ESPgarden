#include <stdint.h>
#include "esp_err.h"

#ifndef AIRSENSORS_H
#define AIRSENSORS_H

esp_err_t getAirData(uint8_t* temp, uint8_t* hum);

esp_err_t airSensorInit();

uint8_t getTemp();

uint8_t getHumidity();

#endif