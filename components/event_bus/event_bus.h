#pragma once
#include "esp_event.h"
#include "esp_err.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// 1) База событий приложения (единая «шина»)
ESP_EVENT_DECLARE_BASE(APP_EVENTS);

// 2) Каталог событий (расширяемый)
typedef enum {
    APP_EVENT_BTN_WIFI_RECONNECT = 0x01,  // Кнопка: просим переподключить Wi-Fi
    APP_EVENT_WIFI_CONNECTED,             // Wi-Fi поднялся
    APP_EVENT_WIFI_DISCONNECTED,          // Wi-Fi упал
    APP_EVENT_WIFI_RECONNECT_REQUEST,     // Доменный запрос на переподключение Wi-Fi
} app_event_id_t;

// Инициализация (опционально; проверяет, что есть дефолтный event loop)
esp_err_t app_events_init(void);

// Публикация событий из задач
static inline esp_err_t app_events_post(app_event_id_t id,
                                        const void *data,
                                        size_t data_size,
                                        TickType_t ticks_to_wait) {
    return esp_event_post(APP_EVENTS, id, data, data_size, ticks_to_wait);
}

// Публикация из ISR
static inline esp_err_t app_events_post_from_isr(app_event_id_t id,
                                                 const void *data,
                                                 size_t data_size,
                                                 BaseType_t *hp_task_woken) {
    return esp_event_isr_post(APP_EVENTS, id, data, data_size, hp_task_woken);
}

// Подписка на конкретный ID (или ESP_EVENT_ANY_ID)
static inline esp_err_t app_events_subscribe(int32_t id,
                                             esp_event_handler_t handler,
                                             void *arg) {
    return esp_event_handler_register(APP_EVENTS, id, handler, arg);
}

// Отписка
static inline esp_err_t app_events_unsubscribe(int32_t id,
                                               esp_event_handler_t handler) {
    return esp_event_handler_unregister(APP_EVENTS, id, handler);
}

#ifdef __cplusplus
}
#endif
