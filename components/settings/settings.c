#include "settings.h"
#include <stdio.h>
SettingsData settings;
// Function to initialize the settings data
void initializeSettings() {
    // Set default values
    settings.lightTime.turnOnHour = 0;
    settings.lightTime.turnOnMinute = 0;
    settings.lightTime.turnOffHour = 0;
    settings.lightTime.turnOffMinute = 0;
    settings.lightTime.durationHours = 0;
    settings.airTemp.MaxTemp = 25;
    settings.airTemp.MinTemp = 20;
}

// Function to update the switch time in the settings data
void updateSwitchTime(uint8_t ONhour, uint8_t OFFhour) {
    // printf("hour: %d\n", hour);
    settings.lightTime.turnOnHour = ONhour;
    settings.lightTime.turnOffHour = OFFhour;
    // printf("updateSwitchTime turnOnHour: %d\n", settings.lightTime.turnOnHour);
    // printf("address settings: %d\n", (int)&settings);
}