#include "settings.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include <stdio.h>

SettingsData settings;

SemaphoreHandle_t lightTimeMutex;
SemaphoreHandle_t airTempMutex;

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

    lightTimeMutex = xSemaphoreCreateMutex();
    airTempMutex = xSemaphoreCreateMutex();
}

// Update light switch time
void updateSwitchTime(uint8_t ONhour, uint8_t OFFhour){
    if (xSemaphoreTake(lightTimeMutex, portMAX_DELAY)) {
        updateTurnONTime(ONhour);
        updateTurnOFFTime(OFFhour);
        
        xSemaphoreGive(lightTimeMutex);
    }
}

void updateTurnONTime(uint8_t ONhour) {
    if (xSemaphoreTake(lightTimeMutex, portMAX_DELAY)) {
        settings.lightTime.turnOnHour = (uint8_t)ONhour;

        xSemaphoreGive(lightTimeMutex);
    }
}

void updateTurnOFFTime(uint8_t OFFhour) {
    if (xSemaphoreTake(lightTimeMutex, portMAX_DELAY)) {
        settings.lightTime.turnOffHour = (uint8_t)OFFhour;

        xSemaphoreGive(lightTimeMutex);
    }
}

void updateMinAirTemp(int min) {
    if (xSemaphoreTake(airTempMutex, portMAX_DELAY)) {
        settings.airTemp.MinTemp = (uint8_t)min;

        xSemaphoreGive(airTempMutex);
    }
}

void updateMaxAirTemp(int max) {
    if (xSemaphoreTake(airTempMutex, portMAX_DELAY)) {
        settings.airTemp.MaxTemp = (uint8_t)max;

        xSemaphoreGive(airTempMutex);
    }
}

void getLightTime(uint8_t* turnOnHour, uint8_t* turnOffHour) {

    if (xSemaphoreTake(lightTimeMutex, portMAX_DELAY)) {
        *turnOnHour = settings.lightTime.turnOnHour;
        *turnOffHour = settings.lightTime.turnOffHour;

        xSemaphoreGive(lightTimeMutex);
    }
}

void getAirTemp(uint8_t* maxTemp, uint8_t* minTemp) {
    if (xSemaphoreTake(airTempMutex, portMAX_DELAY)) {
        *maxTemp = settings.airTemp.MaxTemp;
        *minTemp = settings.airTemp.MinTemp;

        xSemaphoreGive(airTempMutex);
    }
}
