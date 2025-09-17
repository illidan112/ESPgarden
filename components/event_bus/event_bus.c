#include "event_bus.h"
#include "esp_log.h"

ESP_EVENT_DEFINE_BASE(APP_EVENTS);
static const char *TAG = "event_bus";

esp_err_t app_events_init(void) {
    // Защита от отсутствия дефолтного цикла (его создаёт main.c)
    // Если уже создан — функция ничего не делает.
    esp_err_t err = ESP_OK;
#if ESP_IDF_VERSION_MAJOR >= 4
    // Ничего специально создавать не нужно: используем default loop.
    // Добавим лог для диагностики.
    ESP_LOGI(TAG, "event_bus ready (using default event loop)");
#else
#   error "ESP-IDF >= 4 is required for esp_event"
#endif
    return err;
}
