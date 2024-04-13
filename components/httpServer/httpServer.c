/*
 HTTP Server
*/

#include "httpServer.h"

#include "esp_event.h"
#include "esp_netif.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include <esp_http_server.h>
#include <esp_log.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <unistd.h>

#include "airSensor.h"
#include "realtime.h"
#include "settings.h"
#include "wifi.h"
// #include "../../managed_components/espressif__mdns/include/mdns.h"

#if !CONFIG_IDF_TARGET_LINUX
#include "nvs_flash.h"
#include <esp_system.h>
#include <esp_wifi.h>
// #include "esp_eth.h"
#endif // !CONFIG_IDF_TARGET_LINUX

static const char* TAG = "HTTP";
static httpd_handle_t http_server = NULL;

static const uint8_t eventQueueLen = 3;
static QueueHandle_t qServer = NULL;

static bool httpServerState = false;

bool isWebServerRunning() { return httpServerState; }
void setWebServerState(bool flag) { httpServerState = flag; }

void SendServerEvent(const serverEvent event) { xQueueSend(qServer, &event, 0); }
void SendServerEventISR(const serverEvent event) { xQueueSendFromISR(qServer, &event, 0); }

static esp_err_t send_temp_humidity_page(httpd_req_t* req) {
    char html_response[2064]; // Increased size to accommodate additional HTML
    float temperature = getTempFl();
    uint8_t humidity = getHumidity();
    char* dateTimeStr = getStrDateTime(); // Get the date and time string

    // Format the HTML response to include the navigation buttons and temperature, humidity, and date-time values
    int len = snprintf(
        html_response, sizeof(html_response),
        "<!DOCTYPE html>"
        "<html>"

        "<head>"
        "<title>GARDEN</title>"
        "<style>"
        "body { background-color: black; color: white; }"
        "form {text-align: center; margin-top: 20px;}"
        "th, td {border: 1px solid white; text-align: center; padding: 8px;}"
        "th {background-color: #333;}"
        ".nav-table {position: absolute; top: 0; left: 0; width: 20%%;}"
        ".nav-table td {padding: 4px; text-align: center;}"
        ".nav-button {width: 150px; height: 50px;}"
        "table {margin-left: auto; margin-right: auto; margin-top: 20px;}"
        "</style>"
        "</head>"

        "<body>"
        "<table class=\"nav-table\">"
        "<tr><td><form action=\"/\" method=\"get\"><button type=\"submit\" class=\"nav-button\">Main "
        "Page</button></form></td></tr>"
        "<tr><td><form action=\"/settime\" method=\"get\"><button type=\"submit\" class=\"nav-button\">Switching "
        "Time</button></form></td></tr>"
        "<tr><td><form action=\"/settemp\" method=\"get\"><button type=\"submit\" "
        "class=\"nav-button\">Tempreture</button></form></td></tr>"
        "<tr><td><form action=\"/setrtc\" method=\"get\"><button type=\"submit\" "
        "class=\"nav-button\">Clock</button></form></td></tr>"
        "</table>"

        "<h2 style=\"text-align:center;\">GARDEN</h2>"
        "<table>"
        "<tr><th colspan=\"2\">%s</th></tr>"
        "<tr><td>Temperature</td><td>%.1f C</td></tr>"
        "<tr><td>Humidity</td><td>%d%%</td></tr>"
        "</table>"
        "</body>"
        "</html>",
        dateTimeStr, temperature, humidity);

    if (len < 0 || len >= sizeof(html_response)) {
        // Handle error: snprintf failed or buffer size was not enough
        return ESP_ERR_INVALID_SIZE;
    }

    // Use the actual length of the response
    return httpd_resp_send(req, html_response, len);
}

// Function to send settings page for time
static esp_err_t send_rtc_settings_page(httpd_req_t* req) {
    char html_response[1350];

    // Format the HTML response to include a form for time settings
    int len = snprintf(
        html_response, sizeof(html_response),
        "<!DOCTYPE html>"
        "<html>"

        "<head>"
        "<title>Time Settings</title>"
        "<style>"
        "body { background-color: black; color: white; }"
        "form {text-align: center; margin-top: 20px;}"
        "th, td {border: 1px solid white; text-align: center; padding: 8px;}"
        "th {background-color: #333;}"
        ".nav-table {position: absolute; top: 0; left: 0; width: 20%%;}"
        ".nav-table td {padding: 4px; text-align: center;}"
        ".nav-button {width: 150px; height: 50px;}"
        "table {margin-left: auto; margin-right: auto; margin-top: 20px;}"
        "</style>"
        "</head>"

        "<body>"

        "<table class=\"nav-table\">"
        "<tr><td><form action=\"/\" method=\"get\"><button type=\"submit\" class=\"nav-button\">Main "
        "Page</button></form></td></tr>"
        "<tr><td><form action=\"/settime\" method=\"get\"><button type=\"submit\" class=\"nav-button\">Switching "
        "Time</button></form></td></tr>"
        "<tr><td><form action=\"/settemp\" method=\"get\"><button type=\"submit\" "
        "class=\"nav-button\">Tempreture</button></form></td></tr>"
        "<tr><td><form action=\"/setrtc\" method=\"get\"><button type=\"submit\" "
        "class=\"nav-button\">Clock</button></form></td></tr>"
        "</table>"

        "<h2 style=\"text-align:center;\">Clock Settings</h2>"

        "<form action=\"/update_clock\" method=\"post\">"
        "<table align=\"center\">"
        "<tr>"
        "<th>Time</th>"
        "<th>Date</th>"
        "</tr>"
        "<tr>"
        "<td>"
        "<input type=\"time\" id=\"time\" name=\"time\">"
        "</td>"
        "<td>"
        "<input type=\"date\" id=\"date\" name=\"date\">"
        "</td>"
        "</tr>"
        "</table>"
        "<input type=\"submit\" value=\"Submit\">"
        "</form>"

        "</body>"
        "</html>");

    if (len < 0 || len >= sizeof(html_response)) {
        return ESP_ERR_INVALID_SIZE;
    }

    return httpd_resp_send(req, html_response, len);
}

// Function to send settings page for temperature
static esp_err_t send_temperature_settings_page(httpd_req_t* req) {
    char html_response[1590];
    uint8_t maxTemp = 0;
    uint8_t minTemp = 0;

    getAirTemp(&maxTemp, &minTemp);

    // Format the HTML response to include a form for temperature settings
    int len = snprintf(
        html_response, sizeof(html_response),
        "<!DOCTYPE html>"
        "<html>"

        "<head>"
        "<title>Time Settings</title>"
        "<style>"
        "body { background-color: black; color: white; }"
        "form {text-align: center; margin-top: 20px;}"
        "th, td {border: 1px solid white; text-align: center; padding: 8px;}"
        "th {background-color: #333;}"
        ".nav-table {position: absolute; top: 0; left: 0; width: 20%%;}"
        ".nav-table td {padding: 4px; text-align: center;}"
        ".nav-button {width: 150px; height: 50px;}"
        "table {margin-left: auto; margin-right: auto; margin-top: 20px;}"
        "</style>"
        "</head>"

        "<body>"

        "<table class=\"nav-table\">"
        "<tr><td><form action=\"/\" method=\"get\"><button type=\"submit\" class=\"nav-button\">Main "
        "Page</button></form></td></tr>"
        "<tr><td><form action=\"/settime\" method=\"get\"><button type=\"submit\" class=\"nav-button\">Switching "
        "Time</button></form></td></tr>"
        "<tr><td><form action=\"/settemp\" method=\"get\"><button type=\"submit\" "
        "class=\"nav-button\">Tempreture</button></form></td></tr>"
        "<tr><td><form action=\"/setrtc\" method=\"get\"><button type=\"submit\" "
        "class=\"nav-button\">Clock</button></form></td></tr>"
        "</table>"

        "<h2 style=\"text-align:center;\">Temperature Settings</h2>"

        "<table align=\"center\">"
        "<tr>"
        "<th colspan=\"2\">Current setting</th>"
        "</tr>"
        "<tr>"
        "<td class=\"label\">Max temp:</td>"
        "<td class=\"data\">\"%d *C\"</td>"
        "</tr>"
        "<tr>"
        "<td class=\"label\">Min temp:</td>"
        "<td class=\"data\">\"%d *C\"</td>"
        "</tr>"
        "</table>"

        "<form action=\"/update_temp\" method=\"post\">"
        "<table align=\"center\">"
        "<tr>"
        "<th>Max Temp</th>"
        "<th>Min Temp</th>"
        "</tr>"
        "<tr>"
        "<td>"
        "<input type=\"number\" id=\"maxtmp\" name=\"maxtmp\" min=\"0\">"
        "</td>"
        "<td>"
        "<input type=\"number\" id=\"mintmp\" name=\"mintmp\" min=\"0\">"
        "</td>"
        "</tr>"
        "</table>"
        "<input type=\"submit\" value=\"Submit\">"
        "</form>"

        "</body>"
        "</html>",
        maxTemp, minTemp);

    if (len < 0 || len >= sizeof(html_response)) {
        return ESP_ERR_INVALID_SIZE;
    }

    return httpd_resp_send(req, html_response, len);
}

// Function to send settings page for pressure
static esp_err_t send_schdlr_time_sett_page(httpd_req_t* req) {
    char html_response[1650];
    uint8_t onHour = 0;
    uint8_t offHour = 0;

    getLightTime(&onHour, &offHour);

    // Format the HTML response to include a form for pressure settings
    int len = snprintf(
        html_response, sizeof(html_response),
        "<!DOCTYPE html>"
        "<html>"

        "<head>"
        "<title>Time Settings</title>"
        "<style>"
        "body { background-color: black; color: white; }"
        "form {text-align: center; margin-top: 20px;}"
        "th, td {border: 1px solid white; text-align: center; padding: 8px;}"
        "th {background-color: #333;}"
        ".nav-table {position: absolute; top: 0; left: 0; width: 20%%;}"
        ".nav-table td {padding: 4px; text-align: center;}"
        ".nav-button {width: 150px; height: 50px;}"
        "table {margin-left: auto; margin-right: auto; margin-top: 20px;}"
        "</style>"
        "</head>"

        "<body>"

        "<table class=\"nav-table\">"
        "<tr><td><form action=\"/\" method=\"get\"><button type=\"submit\" class=\"nav-button\">Main "
        "Page</button></form></td></tr>"
        "<tr><td><form action=\"/settime\" method=\"get\"><button type=\"submit\" class=\"nav-button\">Switching "
        "Time</button></form></td></tr>"
        "<tr><td><form action=\"/settemp\" method=\"get\"><button type=\"submit\" "
        "class=\"nav-button\">Tempreture</button></form></td></tr>"
        "<tr><td><form action=\"/setrtc\" method=\"get\"><button type=\"submit\" "
        "class=\"nav-button\">Clock</button></form></td></tr>"
        "</table>"

        "<h2 style=\"text-align:center;\">Switching time Settings</h2>"

        "<table align=\"center\">"
        "<tr>"
        "<th colspan=\"2\">Current setting</th>"
        "</tr>"
        "<tr>"
        "<td class=\"label\">Switch ON:</td>"
        "<td class=\"data\">\"%d\"</td>"
        "</tr>"
        "<tr>"
        "<td class=\"label\">Switch OFF:</td>"
        "<td class=\"data\">\"%d\"</td>"
        "</tr>"
        "</table>"

        "<form action=\"/update_schdlr\" method=\"post\">"
        "<table align=\"center\">"
        "<tr>"
        "<th>Turn ON hour</th>"
        "<th>Turn OFF hour</th>"
        "</tr>"
        "<tr>"
        "<td>"
        "<input type=\"number\" id=\"lightOn\" name=\"lightOn\" min=\"0\" max=\"23\">"
        "</td>"
        "<td>"
        "<input type=\"number\" id=\"lightOff\" name=\"lightOff\" min=\"0\" max=\"23\">"
        "</td>"
        "</tr>"
        "</table>"
        "<input type=\"submit\" value=\"Submit\">"
        "</form>"

        "</body>"
        "</html>",
        onHour, offHour);

    if (len < 0 || len >= sizeof(html_response)) {
        return ESP_ERR_INVALID_SIZE;
    }

    return httpd_resp_send(req, html_response, len);
}

static esp_err_t http_404_error_handler(httpd_req_t* req, httpd_err_code_t err) {
    // For any URI send 404 and close socket
    httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "404 Page Not Found");
    return ESP_FAIL;
}

static esp_err_t sendSuccessPage(httpd_req_t* req) {

    char html_response[1024];
    // Format the HTML response
    int len = snprintf(html_response, sizeof(html_response),
                       "<!DOCTYPE html>"
                       "<html>"
                       "<head>"
                       "<title>GARDEN</title>"
                       "<style>"
                       "body { background-color: black; color: white; }"
                       // "form {text-align: center; margin-top: 20px;}"
                       "th, td {border: 1px solid white; text-align: center; padding: 8px;}"
                       "th {background-color: #333;}"
                       ".nav-table {position: absolute; top: 0; left: 0; width: 20%%;}"
                       ".nav-table td {padding: 4px; text-align: center;}"
                       ".nav-button {width: 150px; height: 50px;}"
                       "</style>"
                       "</head>"

                       "<body>"
                       "<table class=\"nav-table\">"
                       "<tr><td><form action=\"/\" method=\"get\"><button type=\"submit\" class=\"nav-button\">Main "
                       "Page</button></form></td></tr>"
                       "</table>"
                       "<h2 style=\"text-align:center;\">Updated succesfully</h2>"

                       "</body>"
                       "</html>");

    if (len < 0 || len >= sizeof(html_response)) {
        // Handle error: snprintf failed or buffer size was not enough
        return ESP_ERR_INVALID_SIZE;
    }

    // Send a response
    // Use the actual length of the response
    return httpd_resp_send(req, html_response, len);
}

// Handler for updating light time
static esp_err_t update_schdlr_post_handle(httpd_req_t* req) {
    char Buf[RESPONSE_BUF_SIZE];
    int ret, received = 0;
    int content_len = req->content_len;

    while (received < content_len) {
        /* Read the data for the request */
        if ((ret = httpd_req_recv(req, Buf + received, MIN(content_len - received, sizeof(Buf) - received))) <= 0) {
            if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
                continue;
            }
            return ESP_FAIL;
        }
        received += ret;
    }
    // Complete string pattern with "&" and Null
    Buf[received] = '&';
    Buf[++received] = '\0';

    // Store new settings in NVS
    const settEvent event = UPDATE_LIGHT_TIME;
    sendRawResponse(Buf);
    SendSettEvent(event);

    return sendSuccessPage(req);
}

// Handler for updating temperature settings
static esp_err_t update_temp_post_handle(httpd_req_t* req) {
    char Buf[RESPONSE_BUF_SIZE];
    int ret, received = 0;
    int content_len = req->content_len;

    while (received < content_len) {
        /* Read the data for the request */
        if ((ret = httpd_req_recv(req, Buf + received, MIN(content_len - received, sizeof(Buf) - received))) <= 0) {
            if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
                continue;
            }
            return ESP_FAIL;
        }
        received += ret;
    }

    // Complete string pattern with "&" and Null
    Buf[received] = '&';
    Buf[++received] = '\0';

    // Store new settings in NVS
    const settEvent event = UPDATE_TEMP;
    sendRawResponse(Buf);
    SendSettEvent(event);

    return sendSuccessPage(req);
}

static esp_err_t update_clock_post_handle(httpd_req_t* req) {
    char Buf[RESPONSE_BUF_SIZE];
    int ret, received = 0;

    int content_len = req->content_len;

    while (received < content_len) {
        /* Read the data for the request */
        if ((ret = httpd_req_recv(req, Buf + received, MIN(content_len - received, sizeof(Buf) - received))) <= 0) {
            if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
                continue;
            }
            return ESP_FAIL;
        }
        received += ret;
    }

    // Null terminate the buffer
    Buf[received] = '&';
    Buf[++received] = '\0';

    // Store new settings in NVS
    const settEvent event = UPDATE_RTC;
    sendRawResponse(Buf);
    SendSettEvent(event);

    return sendSuccessPage(req);
}
/*_________________MAIN_PAGE________________*/
static const httpd_uri_t main = {.uri = "/", .method = HTTP_GET, .handler = send_temp_humidity_page, .user_ctx = NULL};

/*_________________SETTINGS_PAGES________________*/
static const httpd_uri_t sett_rtc_uri = {
    .uri = "/setrtc", .method = HTTP_GET, .handler = send_rtc_settings_page, .user_ctx = NULL};

static const httpd_uri_t sett_temp_uri = {
    .uri = "/settemp", .method = HTTP_GET, .handler = send_temperature_settings_page, .user_ctx = NULL};

static const httpd_uri_t sett_schdlr_time_uri = {
    .uri = "/settime", .method = HTTP_GET, .handler = send_schdlr_time_sett_page, .user_ctx = NULL};

/*_________________UPDATE_HANDLERS_PAGES________________*/
static const httpd_uri_t update_clock_uri = {
    .uri = "/update_clock", .method = HTTP_POST, .handler = update_clock_post_handle, .user_ctx = NULL};

static const httpd_uri_t update_temp_uri = {
    .uri = "/update_temp", .method = HTTP_POST, .handler = update_temp_post_handle, .user_ctx = NULL};

static const httpd_uri_t update_schdlr_uri = {
    .uri = "/update_schdlr", .method = HTTP_POST, .handler = update_schdlr_post_handle, .user_ctx = NULL};

static httpd_handle_t start_webserver(void) {
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server ");
    ESP_LOGI(TAG, "TCP port: '%d'", config.server_port);
    ESP_LOGI(TAG, "UDP port: '%d'", config.ctrl_port);
    if (httpd_start(&server, &config) == ESP_OK) {

        // Set hostname
        //  initialise_mdns();

        // Set URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &main);
        // httpd_register_uri_handler(server, &settings_uri);
        httpd_register_uri_handler(server, &sett_rtc_uri);
        httpd_register_uri_handler(server, &sett_temp_uri);
        httpd_register_uri_handler(server, &sett_schdlr_time_uri);

        httpd_register_uri_handler(server, &update_clock_uri);
        httpd_register_uri_handler(server, &update_schdlr_uri);
        httpd_register_uri_handler(server, &update_temp_uri);
        httpd_register_err_handler(server, HTTPD_404_NOT_FOUND, http_404_error_handler);
        return server;
    }

    ESP_LOGW(TAG, "Error starting server!");
    return NULL;
}

static esp_err_t stop_webserver(httpd_handle_t server) {
    // Stop the httpd server
    return httpd_stop(server);
}

static void connect_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    httpd_handle_t* server = (httpd_handle_t*)arg;
    if (*server == NULL) {
        ESP_LOGI(TAG, "Starting webserver");
        *server = start_webserver();
        if (*server != NULL) {
            httpServerState = true;
        }
    }
}

static void disconnect_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    httpd_handle_t* server = (httpd_handle_t*)arg;
    if (*server) {
        ESP_LOGI(TAG, "Stopping webserver");
        httpServerState = false;
        if (stop_webserver(*server) == ESP_OK) {
            *server = NULL;
        } else {
            ESP_LOGE(TAG, "Failed to stop http server");
        }
    }
}

esp_err_t http_server_start(void) {
    /* Register event handlers to stop the server when Wi-Fi is disconnected,
     * and re-start it upon connection.
     */
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &connect_handler, &http_server));
    ESP_ERROR_CHECK(
        esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disconnect_handler, &http_server));

    ESP_LOGI(TAG, "Init. Handlers registered.");

    return ESP_OK;
}

static void HandleEvent(const serverEvent event) {
    switch (event) {
    case RECONNECT:
        if (!httpServerState) {
            httpServerState = true;
            ESP_LOGI(TAG, "Opening a server");
            wifi_reconnect();
        }
        break;
    }
}

void ServerTask(void* pvParameters) {
    (void)pvParameters;
    wifi_sta_init();
    http_server_start();

    qServer = xQueueCreate(eventQueueLen, sizeof(serverEvent));

    while (1) {
        serverEvent event;
        if (xQueueReceive(qServer, &event, portMAX_DELAY)) {
            HandleEvent(event);
        }
    }
}

// #define ESP_MDNS_URI            "espgarden"
// #define ESP_MDNS_INSTANCE_NAME  "espgarden"

// static void initialise_mdns(void)
// {
//     //initialize mDNS
//     ESP_ERROR_CHECK( mdns_init() );
//     //set mDNS hostname (required if you want to advertise services)
//     ESP_ERROR_CHECK( mdns_hostname_set(ESP_MDNS_URI) );
//     ESP_LOGI("MDNS", "mdns hostname set to: [%s]", ESP_MDNS_URI);
//     //set default mDNS instance name
//     ESP_ERROR_CHECK( mdns_instance_name_set( ESP_MDNS_INSTANCE_NAME) );
// }

// Handler for the main page (GET)
// static esp_err_t main_get_handler(httpd_req_t* req) {
//     const char* resp_str = "<html><body><p style=\"text-align:center\"><u><strong>HI NINOK</strong></u><br />chmok*
//     chmok*</p></body></html>"; httpd_resp_send(req, resp_str, HTTPD_RESP_USE_STRLEN); return ESP_OK;
// }

// static esp_err_t main_template(char* page_code, uint32_t page_size) {
//     char Buf[1024];

//     int len = snprintf(Buf, sizeof(Buf), page_code);

//     if (len < 0 || len >= page_size) {
//         // Handle error: snprintf failed or buffer size was not enough
//         return ESP_ERR_INVALID_SIZE;
//     }

//     len = snprintf(
//         page_code, page_size,
//         "<!DOCTYPE html>"
//         "<html>"

//         "<head>"
//         "<title>GARDEN</title>"
//         "<style>"
//         "body { background-color: black; color: white; }"
//         "table {width: 50%%; margin-left: auto; margin-right: auto; border-collapse: collapse;}"
//         "th, td {border: 1px solid white; text-align: center; padding: 8px;}"
//         "th {background-color: #333;}"
//         ".nav-table {position: absolute; top: 0; left: 0; width: 20%%;}" // Set the width of the left navigation
//         table
//         ".nav-table td {padding: 4px; text-align: center;}"
//         "</style>"
//         "</head>"

//         "<body>"
//         "<table class=\"nav-table\">"
//         "<tr><td><form action=\"/\" method=\"get\"><button type=\"submit\">Main Page</button></form></td></tr>"
//         "<tr><td><form action=\"/settings\" method=\"get\"><button
//         type=\"submit\">Settings</button></form></td></tr>"
//         "</table>"
//         );

//     if (len < 0 || len >= page_size) {
//         // Handle error: snprintf failed or buffer size was not enough
//         return ESP_ERR_INVALID_SIZE;
//     }
//     return ESP_OK;
// }