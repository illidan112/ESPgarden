#include <stdint.h>
#include "esp_err.h"

#ifndef AIRSENSORS_H
#define AIRSENSORS_H

/**
 * @brief Function reads the current air temperature and humidity from BME280, returns the values
 * via pointer arguments and updates global temp and humidity variables
 *
 * @param temp Pointer to a float
 * @param hum  Pointer to a float 
 * @return esp_err_t Returns ESP_OK on success or ESP_FAIL on failure.
 */
esp_err_t getAirData(float* temp, float* hum);

/**
 * @brief Initializes the BME280 sensor module & mutex.
 * @note Checking BME driver with ESP_ERROR_CHECK()
 * 
 * @return esp_err_t Returns ESP_OK.
 */
esp_err_t airSensorInit();

/**
 * @brief Gets the current air temperature and humidity from static variables.
 */
uint8_t getTemp();
uint8_t getHumidity();
float getHumidityFl();
float getTempFl();

/**
 * @brief Gets the current air temp in float by two int pointers and humidity from static variables.
 * 
 * @param intPart   Pointer to a uint8_t for integer part of float temp
 * @param fracPart  Pointer to a uint8_t for FLOAT part of float temp
 */
void getTempDouble(uint8_t *intPart, uint8_t *fracPart);

#endif