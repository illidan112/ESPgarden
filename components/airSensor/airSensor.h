#include <stdint.h>

#ifndef AIRSENSORS_H
#define AIRSENSORS_H

uint8_t getTemp();

uint8_t getHumidity();

void airSensorInit();

#endif