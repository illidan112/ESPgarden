#ifndef _REALTIME_H_
#define _REALTIME_H_

#include <stdio.h>
#include <stdlib.h>
#include <esp_err.h>
#include <time.h>

esp_err_t timeInit();
char* getStrDateTime();

/**
 * @brief Updates the time value on both the DS3231 and the RTC of the ESP32.
 *
 * @param time A pointer to the tm structure with the current time.
 */
void update_rtc(struct tm* time);

/**
 * @brief returns the current hour of day.
 */
uint8_t hoursNow();

#endif
