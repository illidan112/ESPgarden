#include <stdint.h>
#include "esp_err.h"

#ifndef MOVINGFAN_H
#define MOVINGFAN_H

/**
 * @brief Sets the period and on-time duration for the moving fan.
 *
 * This function configures the fan to operate with a specified period and on-time duration.
 * If either the period or on-time duration is 0, the fan will be stopped.
 *
 * @param periodMin The total period for the fan cycle in minutes (1-60).
 * @param onTimeMin The duration in minutes that the fan stays on within the period.
 */
void setMovingFanPeriod(uint8_t periodMin, uint8_t onTimeMin);

#endif