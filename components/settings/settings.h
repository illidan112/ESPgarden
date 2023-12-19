#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include <stdint.h>

typedef struct {
    uint8_t turnOnHour;
    uint8_t turnOnMinute;

    uint8_t turnOffHour;
    uint8_t turnOffMinute;
    uint8_t durationHours;

} LightTime;

typedef struct {
    uint8_t MaxTemp;
    uint8_t MinTemp;

} AirTemp;

typedef struct {
    LightTime lightTime;
    AirTemp airTemp;

} SettingsData;

extern SettingsData settings;

void initializeSettings();

/**
 * @brief update scheduled lighting control time.
 *
 * @param hour hours of turn on light.
 * @param minute minutes of turn on light.
 * @param duration lighting time in hours.
 */
void updateSwitchTime(uint8_t ONhour, uint8_t OFFhour);

#endif
