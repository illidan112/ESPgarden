#include "airSensor.h"
#include "bme280.h"

bme280_data_t bme280_data = {};

void airSensorInit() { bme280_init(); }

uint8_t getHumidity() {
    bme280_get_data(&bme280_data);
    return bme280_data.humidity;
}

uint8_t getTemp() {
    bme280_get_data(&bme280_data);
    return bme280_data.temperature;
}