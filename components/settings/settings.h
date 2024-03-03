#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include <stdint.h>

// Available events in priority order
typedef enum {
    STORE = 0,

} settEvent;

typedef struct {
    uint8_t turnOnHour;
    uint8_t turnOnMinute;

    uint8_t turnOffHour;
    uint8_t turnOffMinute;

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

void SettingsTask(void* pvParameters);
void SendSettEvent(const settEvent event);


/**
 * @brief Update the scheduled times for turning the light on and off.
 *
 * This function updates the global settings for the lighting control system.
 * It sets the hours when the light should be turned on and off.
 *
 * @param ONhour The hour to turn the light on. Range: 0-23
 * @param OFFhour The hour to turn the light off. Range: 0-23
 */
void updateSwitchTime(uint8_t ONhour, uint8_t OFFhour);
void updateTurnOFFTime(uint8_t OFFhour);
void updateTurnONTime(uint8_t ONhour);

/**
 * @brief Update the temperature thresholds for air control.
 *
 * @param max The maximum temperature threshold in Celsius. Range: valid temp range or -1 for no change.
 * @param min The minimum temperature threshold in Celsius. Range: valid temp range or -1 for no change.
 */
void updateMinAirTemp(int min);

void updateMaxAirTemp(int max);

/**
 * @brief Retrieve the current air temperature thresholds.
 *
 * This function reads the global air temperature settings (maximum and minimum thresholds)
 * and stores them in the provided pointer parameters. It ensures thread-safe access to these settings
 * by using mutex locking.
 *
 * @param maxTemp Pointer to store the maximum temperature threshold.
 * @param minTemp Pointer to store the minimum temperature threshold.
 */
void getAirTemp(uint8_t* maxTemp, uint8_t* minTemp);

/**
 * @brief Retrieve the current scheduled times for lighting control.
 *
 * This function reads the global settings for the lighting control times (turn on and off hours)
 * and stores them in the provided pointer parameters. It ensures thread-safe access to these settings
 * by using mutex locking.
 *
 * @param turnOnHour Pointer to store the hour to turn the light on.
 * @param turnOffHour Pointer to store the hour to turn the light off.
 */
void getLightTime(uint8_t* turnOnHour, uint8_t* turnOffHour);

#endif
