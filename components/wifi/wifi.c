/*
    WiFi station
*/

#include "wifi.h"
#include "tController.h"
#include "httpServer.h"

#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include <string.h>

#include "lwip/err.h"
#include "lwip/sys.h"

/*
    Change the below entries to strings with
    the config you want - ie #define  ESP_WIFI_SSID "mywifissid"
*/
// #define ESP_WIFI_SSID "TrueWIFI"
// #define ESP_WIFI_PASS "qwerty4321"
#define ESP_WIFI_SSID "iPhone (Ilya)"
#define ESP_WIFI_PASS "Abobaaboba112"
#define ESP_MAXIMUM_RETRY 5

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

static const char* TAG = "WIFI";

static int s_retry_num = 0;
static esp_netif_t* sta_netif = NULL;
static bool isConnected = false;

bool isWifiConnected() { return isConnected; }

static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
            ESP_LOGW(TAG, "Failed to connect to SSID:%s, password:%s", ESP_WIFI_SSID, ESP_WIFI_PASS);
            isConnected = false;
            
            serverEvent event = STOP;
            SendServerEvent(event);
        }
        // ESP_LOGI(TAG,"connect to the AP fail");
    } else if (event_base == IP_EVENT) {
        // printf("ID event: %d\n", (int)event_id);
        switch (event_id) {
        case IP_EVENT_STA_GOT_IP: {
            ip_event_got_ip_t* event = (ip_event_got_ip_t*)event_data;
            ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
            s_retry_num = 0;
            xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
            ESP_LOGI(TAG, "connected to ap SSID:%s password:%s", ESP_WIFI_SSID, ESP_WIFI_PASS);
            isConnected = true;
            break;
        }
            // default:{
            //     printf("ID event: %d\n", (int)event_id);
            // }
        }
    }
}

esp_err_t wifi_sta_init(void) {

    esp_err_t ret_value = ESP_OK;
    s_wifi_event_group = xEventGroupCreate();

    sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    cfg.wifi_task_core_id = 0U;
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_LOST_IP, &event_handler, NULL));

    wifi_config_t wifi_config = {
        .sta = {.ssid = ESP_WIFI_SSID, .password = ESP_WIFI_PASS},
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits =
        xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdFALSE, pdFALSE, 1000000);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGW(TAG, "CoreID: %d", xPortGetCoreID());
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s", ESP_WIFI_SSID, ESP_WIFI_PASS);
        isConnected = true;
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGW(TAG, "Returned: Failed to connect(ESP_FAIL)");
        isConnected = false;
        ret_value = ESP_FAIL;
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
        isConnected = false;
        ret_value = ESP_ERR_INVALID_STATE;
    }

    return ret_value;
}

esp_err_t wifi_sta_stop() {

    ESP_LOGI(TAG, "Stop WIFI");
    isConnected = false;

    // deinit wifi_event_group
    if (esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler) ||
        esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler) ||
        esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_LOST_IP, &event_handler) == ESP_FAIL) {
        ESP_LOGE(TAG, "deinit wifi_event_group error");
        return ESP_FAIL;
    }

    xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT);
    vEventGroupDelete(s_wifi_event_group);
    s_wifi_event_group = NULL;

    // deinit sta_netif for esp_netif_create_default_wifi_sta()
    esp_netif_destroy(sta_netif);

    // ESP_ERROR_CHECK(esp_wifi_deinit());
    // ESP_ERROR_CHECK(esp_wifi_stop());

    if (esp_wifi_stop() == ESP_FAIL) {
        ESP_LOGE(TAG, "Error during stop wi-fi");
        return ESP_FAIL;
    }

    return ESP_OK;
}
