#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "servo.h"
#include "GPIO.h"
#include "movingFan.h"

static TimerHandle_t fanTimer;
static uint32_t onTimeMs = 0;
static uint32_t offTimeMs = 0;
static bool isFanOn = false;

void startMovingFan() { 
    fanTurnON(movingFan); 
    resumeRotation();
}

void stopMovingFan() { 
    fanTurnOFF(movingFan); 
    suspendRotation();
}

void movFanTmrCallback(TimerHandle_t xTimer) {
    if (isFanOn) {
        isFanOn = false;
        stopMovingFan();
        xTimerChangePeriod(fanTimer, offTimeMs, 0);
        xTimerStart(fanTimer, 0);
    } else {
        isFanOn = true;
        xTimerChangePeriod(fanTimer, onTimeMs, 0);
        xTimerStart(fanTimer, 0);
        startMovingFan();
    }
}

esp_err_t initMovingFan() {
    fanTimer = xTimerCreate("Moving Fan timer", pdMS_TO_TICKS(1), pdFALSE, 0, movFanTmrCallback);
    if (fanTimer == NULL) {
        return ESP_FAIL;
    }
    return ESP_OK;
}

void setMovingFanPeriod(uint8_t periodMin, uint8_t onTimeMin) {

    // disabling fan
    if(periodMin == 0 || onTimeMin == 0){
        stopMovingFan();
        xTimerStop(fanTimer, 0);
        isFanOn = false;
        return;
    }

    // configuring fan
    if (periodMin > 60) {
        periodMin = 60;
    }

    if (onTimeMin >= periodMin){
        onTimeMin = periodMin;
    }
    onTimeMs = pdMS_TO_TICKS(onTimeMin * 1000 * 60); 
    offTimeMs = pdMS_TO_TICKS((periodMin - onTimeMin) * 1000 * 60);

    // enabling fan after configuration
    if (xTimerIsTimerActive(fanTimer) == pdTRUE) {
        xTimerStop(fanTimer, 0);
    }

    xTimerChangePeriod(fanTimer, onTimeMs, 0);
    xTimerStart(fanTimer, 0);
    startMovingFan();
    isFanOn = true;

}
