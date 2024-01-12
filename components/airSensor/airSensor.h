#include <stdint.h>
#include "esp_err.h"

#ifndef AIRSENSORS_H
#define AIRSENSORS_H

esp_err_t getAirData(int32_t* temp, uint32_t* hum);

esp_err_t airSensorInit();

uint8_t getTemp();

uint8_t getHumidity();

#endif