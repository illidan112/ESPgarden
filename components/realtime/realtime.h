#ifndef _REALTIME_H_
#define _REALTIME_H_

#include <stdio.h>
#include <stdlib.h>
#include <esp_err.h>

void stringDateTime();
esp_err_t timeInit();

/**
 * @brief returns the current hour of day.
 */
uint8_t hoursNow();

#endif
