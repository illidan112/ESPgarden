#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include <ds3231.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#include "realtime.h"

#define COMPILATION_TIME __TIME__
#define DATA_TIME_SIZE 24

// set ds3231 time on startup
#define SET_DS3231_TIME 0

#define I2C_MASTER_SDA 21
#define I2C_MASTER_SCL 22

const static char* TAG = "RTC";
char dataTimeStr[DATA_TIME_SIZE];
SemaphoreHandle_t timeMutex;
static i2c_dev_t ds3231Desc;

esp_err_t init_ds3231(gpio_num_t sda_pin, gpio_num_t scl_pin) {
    memset(&ds3231Desc, 0, sizeof(i2c_dev_t));
    return ds3231_init_desc(&ds3231Desc, I2C_NUM_0, sda_pin, scl_pin);
}

esp_err_t get_time_ds3231(struct tm* time) { 
    if(time != NULL){
        return ds3231_get_time(&ds3231Desc, time);
    }
    return ESP_FAIL;
}

esp_err_t set_time_ds3231(struct tm* time) { return ds3231_set_time(&ds3231Desc, time); }

esp_err_t timeInit() {

    const char* compilationDateTime = __DATE__ " " __TIME__;
    // char strftime_buf[64];
    struct timeval tv;
    struct tm tm_info;
    // memset(&tm_info, 0, sizeof(struct tm)); // reset to zero

    // DS32321 INIT
    if (init_ds3231(I2C_MASTER_SDA, I2C_MASTER_SCL) != ESP_OK) {
        ESP_LOGE(TAG, "cant init ds3231");
        return ESP_FAIL;
    }

    vTaskDelay(pdMS_TO_TICKS(250));

    if (SET_DS3231_TIME) {
        // Set timezone
        setenv("TZ", "UTC+2", 1);
        tzset();

        if (strptime(compilationDateTime, "%b %e %Y %H:%M:%S", &tm_info) == NULL) {
            ESP_LOGE(TAG, "Time string conversion error");
            return ESP_FAIL;
        }

        ESP_ERROR_CHECK(set_time_ds3231(&tm_info));
        // memset(&tm_info, 0, sizeof(struct tm)); // reset to zero
    }

    // Get current time from DS3231
    if (get_time_ds3231(&tm_info) != ESP_OK) {
        ESP_LOGE(TAG, "Cant get time from DS3231");
        return ESP_FAIL;
    }

    // translate tm_info in Unix epoch
    tv.tv_sec = mktime(&tm_info);
    tv.tv_usec = 0;

    // ESP RTC time updates
    settimeofday(&tv, NULL);

    timeMutex = xSemaphoreCreateMutex();

    return ESP_OK;
}

uint8_t hoursNow() {

    time_t timeNow;
    struct tm timeinfo;

    if (xSemaphoreTake(timeMutex, portMAX_DELAY)) {
        time(&timeNow);
        localtime_r(&timeNow, &timeinfo);

        xSemaphoreGive(timeMutex);
    }
    return timeinfo.tm_hour;
}

// void stringDateTime() {

//     time_t now;
//     char strftime_buf[64];
//     struct tm timeinfo;

//     time(&now);
//     localtime_r(&now, &timeinfo);
//     strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
//     ESP_LOGI(TAG, "The current date/time is: %s", strftime_buf);
// }

char* getStrDateTime() {
    time_t now;
    struct tm timeinfo;

    if (xSemaphoreTake(timeMutex, portMAX_DELAY)) {
        time(&now);
        localtime_r(&now, &timeinfo);
        strftime(dataTimeStr, 9, "%x", &timeinfo);     // getting date in format DD/MM/YY
        dataTimeStr[8] = ' ';                          // erase '\0' symbol
        strftime(dataTimeStr + 9, 9, "%X", &timeinfo); // getting time in the same array in format HH:MM:SS
        xSemaphoreGive(timeMutex);
    }

    return dataTimeStr;
}
