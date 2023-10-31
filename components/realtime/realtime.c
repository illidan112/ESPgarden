
#include "realtime.h"

// struct timeval tv = { 1698099699, 0 };
// settimeofday(&tv, NULL);

// while(1){

// // time_t now;
// // char strftime_buf[64];
// // struct tm timeinfo;

// // time(&now);
// // // Set timezone to China Standard Time
// // setenv("TZ", "UTC+2", 1);
// // tzset();

// // localtime_r(&now, &timeinfo);
// // strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);

// time_t current_time;
// struct tm *time_info;
// int hours, minutes;

// time(&current_time);
// time_info = localtime(&current_time);

// hours = time_info->tm_hour;
// minutes = time_info->tm_min;
// ESP_LOGI(TAG, "Current time: %02d:%02d", hours, minutes);
// // test();
// vTaskDelay(pdMS_TO_TICKS(1000));
// }

void stringTime() { printf("Скомпилировано %s в %s\n", __DATE__, __TIME__); }