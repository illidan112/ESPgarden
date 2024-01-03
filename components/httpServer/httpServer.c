/*
 HTTP Server
*/

#include "httpServer.h"

#include "esp_event.h"
#include "esp_netif.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include <esp_http_server.h>
#include <esp_log.h>
#include <nvs_flash.h>
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

static const char* TAG = "http_server";
static httpd_handle_t http_server = NULL;

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
//     char buf[1024];

//     int len = snprintf(buf, sizeof(buf), page_code);

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
//         "form {text-align: center; margin-top: 20px;}"
//         "th, td {border: 1px solid white; text-align: center; padding: 8px;}"
//         "th {background-color: #333;}"
//         ".nav-table {position: absolute; top: 0; left: 0; width: 20%%;}"
//         ".nav-table td {padding: 4px; text-align: center;}"
//         "</style>"
//         "</head>"

//         "<body>"
//         "<table class=\"nav-table\">"
//         "<tr><td><form action=\"/\" method=\"get\"><button type=\"submit\">Main Page</button></form></td></tr>"
//         "<tr><td><form action=\"/settings\" method=\"get\"><button type=\"submit\">Settings</button></form></td></tr>"
//         "</table>%s"

//         "</html>",
//         buf);

//     if (len < 0 || len >= page_size) {
//         // Handle error: snprintf failed or buffer size was not enough
//         return ESP_ERR_INVALID_SIZE;
//     }
//     return ESP_OK;
// }

// static esp_err_t send_temp_humidity_page(httpd_req_t* req) {
//     char html_response[2048]; // Increased size to accommodate additional HTML
//     uint8_t temperature = getTemp();
//     uint8_t humidity = getHumidity();
//     char* dateTimeStr = getStrDateTime(); // Get the date and time string

//     // Format the HTML response to include the navigation buttons and temperature, humidity, and date-time values
//     int len = snprintf(
//         html_response, sizeof(html_response),
//         "<body>"
//         "<table class=\"nav-table\">"
//         "<tr><td><form action=\"/\" method=\"get\"><button type=\"submit\">Main Page</button></form></td></tr>"
//         "<tr><td><form action=\"/settings\" method=\"get\"><button type=\"submit\">Settings</button></form></td></tr>"
//         "</table>"
//         "<h2 style=\"text-align:center;\">GARDEN</h2>"
//         "<table>"
//         "<tr><th colspan=\"2\">%s</th></tr>"
//         "<tr><td>Temperature</td><td>%d°C</td></tr>"
//         "<tr><td>Humidity</td><td>%d%%</td></tr>"
//         "</table>"
//         "</body>",
//         dateTimeStr, temperature, humidity);

//     if (len < 0 || len >= sizeof(html_response)) {
//         // Handle error: snprintf failed or buffer size was not enough
//         return ESP_ERR_INVALID_SIZE;
//     }

//     main_template(html_response, sizeof(html_response));

//     // Use the actual length of the response
//     return httpd_resp_send(req, html_response, len);
// }

static esp_err_t send_temp_humidity_page(httpd_req_t* req) {
    char html_response[2048]; // Increased size to accommodate additional HTML
    uint8_t temperature = getTemp();
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
        "table {width: 50%%; margin-left: auto; margin-right: auto; border-collapse: collapse;}"
        "th, td {border: 1px solid white; text-align: center; padding: 8px;}"
        "th {background-color: #333;}"
        ".nav-table {position: absolute; top: 0; left: 0; width: 20%%;}" // Set the width of the left navigation table
        ".nav-table td {padding: 4px; text-align: center;}" // Adjust padding and text alignment for table cells
        "</style>"
        "</head>"
        "<body>"
        "<table class=\"nav-table\">"
        "<tr><td><form action=\"/\" method=\"get\"><button type=\"submit\">Main Page</button></form></td></tr>"
        "<tr><td><form action=\"/settings\" method=\"get\"><button type=\"submit\">Settings</button></form></td></tr>"
        "</table>"
        "<h2 style=\"text-align:center;\">GARDEN</h2>"
        "<table>"
        "<tr><th colspan=\"2\">%s</th></tr>"
        "<tr><td>Temperature</td><td>%d°C</td></tr>"
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

static esp_err_t send_settings_page(httpd_req_t* req) {
    char html_response[1500];

    // Format the HTML response to include a form with a dropdown and a button
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
        "table {margin-left: auto; margin-right: auto; margin-top: 20px;}"
        "</style>"
        "</head>"

        "<body>"
        "<table class=\"nav-table\">"
        "<tr><td><form action=\"/\" method=\"get\"><button type=\"submit\">Main Page</button></form></td></tr>"
        "<tr><td><form action=\"/settings\" method=\"get\"><button type=\"submit\">Settings</button></form></td></tr>"
        "</table>"
        "<h2 style=\"text-align:center;\">Select Settings</h2>"

        "<form action=\"/update\" method=\"post\">"
        "<table>"
        "<tr><th>Air temp</th><th>Light time</th></tr>"
        "<tr><td>MAX temp: <input type=\"number\" id=\"maxTemp\" name=\"maxTemp\" min=\"0\" title=\"Temp for turn on "
        "box fan\"></td>"
        "<td>Light on: <input type=\"number\" id=\"lightOn\" name=\"lightOn\" min=\"0\" max=\"23\"></td></tr>"

        "<tr><td>Min temp: <input type=\"number\" id=\"minTemp\" name=\"minTemp\" min=\"0\"></td>"
        "<td>Light off: <input type=\"number\" id=\"lightOff\" name=\"lightOff\" min=\"0\" max=\"23\"></td></tr>"
        "</table>"
        "<input type=\"submit\" value=\"Submit\">"
        "</form>"

        "</body>"
        "</html>");

    if (len < 0 || len >= sizeof(html_response)) {
        // Handle error: snprintf failed or buffer size was not enough
        return ESP_ERR_INVALID_SIZE;
    }

    // Use the actual length of the response
    return httpd_resp_send(req, html_response, len);
}

esp_err_t http_404_error_handler(httpd_req_t* req, httpd_err_code_t err) {
    // For any URI send 404 and close socket
    httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "404 Page Not Found");
    return ESP_FAIL;
}

static esp_err_t handle_update_post(httpd_req_t* req) {
    char buf[256];
    int ret, received = 0;
    // char maxTemp[32] = {0}, minTemp[32] = {0}, lightOn[32] = {0}, lightOff[32] = {0};
    // char number_value[32] = {0}; // Buffer to store the number value
    int maxTemp, minTemp, lightOn, lightOff;

    int content_len = req->content_len;

    while (received < content_len) {
        /* Read the data for the request */
        if ((ret = httpd_req_recv(req, buf + received, MIN(content_len - received, sizeof(buf) - received))) <= 0) {
            if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
                continue;
            }
            return ESP_FAIL;
        }
        received += ret;
    }

    // Null terminate the buffer
    buf[received] = '\0';

    // Function to extract data from the buffer
    int8_t extract_data(const char* field_name) {
        char output[16];
        char* start = strstr(buf, field_name);
        if (start) {
            start += strlen(field_name);    // Move start pointer to the value
            char* end = strstr(start, "&"); // Find the end of the value
            if (end == NULL) {
                end = buf + received; // If there's no other parameter, end is at the end of the data
            }
            int len = MIN(end - start, sizeof(output) - 1);
            if (len == 0){
                return -1;
            }
            printf("len:%d", len);
            strncpy(output, start, len);
            output[len] = '\0'; // Null terminate the copied value
            return atoi(output);
        }
        return -1;
    }

    // Extract values from all fields
    // extract_data("maxTemp=", maxTemp, sizeof(maxTemp));
    // extract_data("minTemp=", minTemp, sizeof(minTemp));
    // extract_data("lightOn=", lightOn, sizeof(lightOn));
    // extract_data("lightOff=", lightOff, sizeof(lightOff));



    maxTemp = extract_data("maxTemp=");
    if(maxTemp >= 0){
        updateMaxAirTemp(maxTemp);
    }

    minTemp = extract_data("minTemp=");
    if(minTemp >= 0){
        updateMinAirTemp(minTemp);
    }

    lightOn = extract_data("lightOn=");
    if(lightOn >= 0){
        updateTurnONTime(lightOn);
    }

    lightOff = extract_data("lightOff=");
    if(lightOff >= 0){
        updateTurnOFFTime(lightOff);
    }

    // Log the values (or handle them as needed)
    // ESP_LOGI(TAG, "Max Temp: %s, Min Temp: %s, Light On: %s, Light Off: %s", maxTemp, minTemp, lightOn, lightOff);
    ESP_LOGI(TAG, "Max Temp: %d, Min Temp: %d, Light On: %d, Light Off: %d", maxTemp, minTemp, lightOn, lightOff);

    char html_response[1024];
    // Format the HTML response
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
        "</style>"
        "</head>"

        "<body>"
        "<table class=\"nav-table\">"
        "<tr><td><form action=\"/\" method=\"get\"><button type=\"submit\">Main Page</button></form></td></tr>"
        "<tr><td><form action=\"/settings\" method=\"get\"><button type=\"submit\">Settings</button></form></td></tr>"
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

static const httpd_uri_t main = {.uri = "/", .method = HTTP_GET, .handler = send_temp_humidity_page, .user_ctx = NULL};

static const httpd_uri_t settings_uri = {
    .uri = "/settings", .method = HTTP_GET, .handler = send_settings_page, .user_ctx = NULL};

static const httpd_uri_t update = {
    .uri = "/update", .method = HTTP_POST, .handler = handle_update_post, .user_ctx = NULL};

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
        httpd_register_uri_handler(server, &settings_uri);
        httpd_register_uri_handler(server, &update);
        httpd_register_err_handler(server, HTTPD_404_NOT_FOUND, http_404_error_handler);
        return server;
    }

    ESP_LOGI(TAG, "Error starting server!");
    return NULL;
}

static esp_err_t stop_webserver(httpd_handle_t server) {
    if (server) {
        return httpd_stop(server);
    }
    return ESP_OK;
}

static void disconnect_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    httpd_handle_t* server = (httpd_handle_t*)arg;
    if (*server) {
        ESP_LOGI(TAG, "Stopping webserver");
        if (stop_webserver(*server) != ESP_OK) {
            ESP_LOGE(TAG, "Failed to stop http server");
        }
        *server = NULL;
        http_server = NULL;
    }
}

static void connect_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    httpd_handle_t* server = (httpd_handle_t*)arg;
    if (*server == NULL) {
        ESP_LOGI(TAG, "Starting webserver");
        *server = start_webserver();
    }
}

bool isHttpServerActive() {
    if (http_server == NULL) {
        return false;
    }
    return true;
}

esp_err_t http_server_start(void) {

    if (isWifiConnected()) {

        // static httpd_handle_t server = NULL;

        /* Register event handlers to stop the server when Wi-Fi is disconnected,
         * and re-start it upon connection.
         */
        ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &connect_handler, &http_server));
        ESP_ERROR_CHECK(
            esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disconnect_handler, &http_server));

        /* Start the server for the first time */
        http_server = start_webserver();

        return ESP_OK;
    } else {
        ESP_LOGW(TAG, "WI-FI disconnected");
        return ESP_FAIL;
    }
}