#include "settings.h"
#include <stdio.h>
SettingsData settings;
// Function to initialize the settings data
void initializeSettings() {
    // Set default values
    settings.lightTime.turnOnHour = 0;
    settings.lightTime.turnOnMinute = 0;
    settings.lightTime.durationHours = 0;
    settings.airTemp.MaxTemp = 25;
    settings.airTemp.MinTemp = 20;
}

// Function to update the switch time in the settings data
void updateSwitchTime(uint8_t hour, uint8_t minute, uint8_t duration) {
    // printf("hour: %d\n", hour);
    settings.lightTime.turnOnHour = hour;
    settings.lightTime.turnOnMinute = minute;
    settings.lightTime.durationHours = duration;
    // printf("updateSwitchTime turnOnHour: %d\n", settings.lightTime.turnOnHour);
    // printf("address settings: %d\n", (int)&settings);
}