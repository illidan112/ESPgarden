#include "movingFan.h"
#include <FreeRTOS.h>
// #include <task.h>
#include "GPIO.h"
#include <timers.h>

static TimerHandle_t fanTimer;

void movFanTmrCallback() {}

esp_err_t initMovingFan() {

    fanTimer = xTimerCreate("Moving Fan timer", 1, pdTRUE, 0, movFanTmrCallback);
    if (fanTimer == NULL) {
        return ESP_FAIL;
    }
    return ESP_OK;

}

void startMovingFan() { fanTurnON(movingFan); }

void stopMovingFan() { fanTurnOFF(movingFan); }

void setMovingFanPeriod() {}

// #include "movingFan.h"
// #include <FreeRTOS.h>
// #include "GPIO.h"
// #include <timers.h>

// static TimerHandle_t fanTimer;
// static uint32_t onTime = 0;
// static uint32_t offTime = 0;
// static bool isFanOn = false;

// void movFanTmrCallback(TimerHandle_t xTimer) {
//     if (isFanOn) {
//         stopMovingFan();
//         isFanOn = false;
//         xTimerChangePeriod(fanTimer, offTime, 0);
//     } else {
//         startMovingFan();
//         isFanOn = true;
//         xTimerChangePeriod(fanTimer, onTime, 0);
//     }
// }

// esp_err_t initMovingFan() {
//     fanTimer = xTimerCreate("Moving Fan timer", pdMS_TO_TICKS(1000), pdFALSE, 0, movFanTmrCallback);
//     if (fanTimer == NULL) {
//         return ESP_FAIL;
//     }
//     return ESP_OK;
// }

// void startMovingFan() { 
//     fanTurnON(movingFan); 
// }

// void stopMovingFan() { 
//     fanTurnOFF(movingFan); 
// }

// void setMovingFanPeriod(uint8_t minutesPerHour) {
//     if (minutesPerHour > 60) {
//         minutesPerHour = 60; // максимальное время работы - 60 минут в час
//     }
//     onTime = pdMS_TO_TICKS(minutesPerHour * 60000); // время включения в миллисекундах
//     offTime = pdMS_TO_TICKS((60 - minutesPerHour) * 60000); // время выключения в миллисекундах

//     if (xTimerIsTimerActive(fanTimer) == pdTRUE) {
//         xTimerStop(fanTimer, 0);
//     }

//     isFanOn = false;
//     xTimerChangePeriod(fanTimer, onTime, 0);
//     xTimerStart(fanTimer, 0);
// }
