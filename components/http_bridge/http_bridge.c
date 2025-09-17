#include "http_bridge.h"
#include "event_bus.h"
#include "http_server.h"  // реализацию сервера НЕ меняем — только вызываем его API
#include "esp_log.h"
#include "wifi.h"

static const char *TAG = "http_bridge";

// Единый обработчик событий приложения
static void on_app_event(void *arg, esp_event_base_t base, int32_t id, void *data) {
    (void)arg; (void)base; (void)data;

    switch (id) {
        case APP_EVENT_BTN_WIFI_RECONNECT:
        case APP_EVENT_WIFI_RECONNECT_REQUEST:
            // Проксируем прежний вызов в веб-сервер без изменения его кода
            // SendServerEvent(RECONNECT);
            wifi_reconnect();
            ESP_LOGI(TAG, "forwarded RECONNECT to http_server");
            break;

        case APP_EVENT_WIFI_CONNECTED:
            setWebServerState(true);
            ESP_LOGI(TAG, "set web state: RUNNING");
            break;

        case APP_EVENT_WIFI_DISCONNECTED:
            setWebServerState(false);
            ESP_LOGI(TAG, "set web state: STOPPED");
            break;

        default:
            break;
    }
}

esp_err_t http_bridge_init(void) {
    // Подписываемся сразу на ВСЕ события и фильтруем по switch
    ESP_ERROR_CHECK(app_events_subscribe(ESP_EVENT_ANY_ID, on_app_event, NULL));
    return ESP_OK;
}
