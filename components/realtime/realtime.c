#include "esp_log.h"
#include <string.h>
#include <sys/time.h>
#include <time.h>

#include "realtime.h"

#define COMPILATION_TIME __TIME__

const static char* TAG = "RTC";

esp_err_t timeInit() {

    const char* compilationDateTime = __DATE__ " " __TIME__;
    // char strftime_buf[64];
    struct timeval tv;
    struct tm tm_info;
    memset(&tm_info, 0, sizeof(struct tm));

    // Set timezone
    setenv("TZ", "UTC+2", 1);
    tzset();

    if (strptime(compilationDateTime, "%b %e %Y %H:%M:%S", &tm_info) == NULL) {
        ESP_LOGE(TAG, "Time string conversion error");
        return ESP_FAIL;
    }

    // translate tm_info in Unix epoch
    tv.tv_sec = mktime(&tm_info);
    tv.tv_usec = 0;
    // ESP RTC time updates
    settimeofday(&tv, NULL);

    return ESP_OK;
}

uint8_t hoursNow() {

    time_t timeNow;
    struct tm timeinfo;

    time(&timeNow);
    localtime_r(&timeNow, &timeinfo);

    return timeinfo.tm_hour;
}

void stringDateTime() {

    time_t now;
    char strftime_buf[64];
    struct tm timeinfo;

    time(&now);
    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGI(TAG, "The current date/time is: %s", strftime_buf);
}
