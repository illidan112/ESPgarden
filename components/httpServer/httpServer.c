/*
 HTTP Server
*/

#include "httpServer.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "esp_event.h"
#include "esp_netif.h"
#include <esp_http_server.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <unistd.h>
// #include "../../managed_components/espressif__mdns/include/mdns.h"

#if !CONFIG_IDF_TARGET_LINUX
#include <esp_wifi.h>
#include <esp_system.h>
#include "nvs_flash.h"
// #include "esp_eth.h"
#endif  // !CONFIG_IDF_TARGET_LINUX

static const char* TAG = "http_server";
static httpd_handle_t main_server = NULL;

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
static esp_err_t main_get_handler(httpd_req_t* req) {
    const char* resp_str = "<html><body><p style=\"text-align:center\"><u><strong>HI NINOK</strong></u><br />chmok* chmok*</p></body></html>";
    httpd_resp_send(req, resp_str, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static esp_err_t send_temp_humidity_page(httpd_req_t *req) {

    // Use snprintf instead of strcpy and strncpy for safer and more efficient code
    char html_response[1024];
    u_int8_t temperature = 32;
    u_int8_t humidity = 54;

    // Directly format the HTML response with temperature and humidity values
    // This removes the need for temporary strings and multiple strncpy calls
    int len = snprintf(html_response, sizeof(html_response),
        "<!DOCTYPE html>"
        "<html>"
        "<head>"
        "<title>ESP32 Sensor Data</title>"
        "<style>"
        "body { background-color: black; color: white; }"
        "table {width: 50%%; margin-left: auto; margin-right: auto; border-collapse: collapse;}"
        "th, td {border: 1px solid white; text-align: center; padding: 8px;}"
        "th {background-color: #333;}"
        "</style>"
        "</head>"
        "<body>"
        "<h2 style=\"text-align:center;\">Temperature and Humidity</h2>"
        "<table>"
        "<tr><th>Measurement</th><th>Value</th></tr>"
        "<tr><td>Temperature</td><td>%d°C</td></tr>"
        "<tr><td>Humidity</td><td>%d%%</td></tr>"
        "</table>"
        "</body>"
        "</html>", temperature, humidity);

    if (len < 0 || len >= sizeof(html_response)) {
        // Handle error: snprintf failed or buffer size was not enough
        return ESP_ERR_INVALID_SIZE;
    }

    // Use the actual length of the response
    return httpd_resp_send(req, html_response, len);
}

// Handler for the settings page (POST)
// static esp_err_t settings_post_handler(httpd_req_t* req) {
//     // char buf[100];
//     // int ret, remaining = req->content_len;

//     // while (remaining > 0) {
//     //     if ((ret = httpd_req_recv(req, buf, MIN(remaining, sizeof(buf)))) <= 0) {
//     //         if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
//     //             continue;
//     //         }
//     //         return ESP_FAIL;
//     //     }

//     //     // Process received data
//     //     ESP_LOGI(TAG, "Received data: %.*s", ret, buf);

//     //     remaining -= ret;
//     // }

//     // // Send response
//     // const char* resp_str = "Settings updated";
//     // httpd_resp_send(req, resp_str, HTTPD_RESP_USE_STRLEN);
//     // return ESP_OK;

//     /* Destination buffer for content of HTTP POST request.
//      * httpd_req_recv() accepts char* only, but content could
//      * as well be any binary data (needs type casting).
//      * In case of string data, null termination will be absent, and
//      * content length would give length of string */

//     char content[100];

//     /* Truncate if content length larger than the buffer */
//     size_t recv_size = MIN(req->content_len, sizeof(content));

//     int ret = httpd_req_recv(req, content, recv_size);
//     if (ret <= 0) {  /* 0 return value indicates connection closed */
//         /* Check if timeout occurred */
//         if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
//             /* In case of timeout one can choose to retry calling
//              * httpd_req_recv(), but to keep it simple, here we
//              * respond with an HTTP 408 (Request Timeout) error */
//             httpd_resp_send_408(req);
//         }
//         /* In case of error, returning ESP_FAIL will
//          * ensure that the underlying socket is closed */
//         return ESP_FAIL;
//     }

//     /* Send a simple response */
//     const char resp[] = "URI POST Response";
//     httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
//     return ESP_OK;
// }

static esp_err_t send_dropdown_page(httpd_req_t *req) {
    char html_response[1024];

    // Format the HTML response to include a form with a dropdown and a button
    int len = snprintf(html_response, sizeof(html_response),
        "<!DOCTYPE html>"
        "<html>"
        "<head>"
        "<title>ESP32 Control Panel</title>"
        "<style>"
        "body { background-color: black; color: white; }"
        "form {text-align: center; margin-top: 20px;}"
        "</style>"
        "</head>"
        "<body>"
        "<h2 style=\"text-align:center;\">Select Settings</h2>"
        "<form action=\"/update\" method=\"post\">"
        "<label for=\"setting\">Choose a setting:</label>"
        "<select name=\"setting\" id=\"setting\">"
        "<option value=\"option1\">Option 1</option>"
        "<option value=\"option2\">Option 2</option>"
        "<option value=\"option3\">Option 3</option>"
        "</select>"
        "<br><br>"
        "<input type=\"submit\" value=\"Update\">"
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

static esp_err_t handle_update_post(httpd_req_t *req) {
    char buf[100];
    int ret, received = 0;
    char setting_value[32] = {0};  // Buffer to store the setting value

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

    // Extract the setting value from the buffer
    // Assuming the form data is sent as 'setting=selected_option'
    char *start = strstr(buf, "setting=");
    if (start) {
        start += strlen("setting=");  // Move start pointer to the value
        char *end = strstr(start, "&");  // Find the end of the value
        if (end == NULL) {
            end = buf + received;  // If there's no other parameter, end is at the end of the data
        }
        int len = MIN(end - start, sizeof(setting_value) - 1);
        strncpy(setting_value, start, len);
        setting_value[len] = '\0';  // Null terminate the copied value
    }

    // You now have the selected setting in setting_value
    // Add your logic here to handle the setting

    // ESP_LOGI(TAG,setting_value);
    ESP_LOGI(TAG,"OPT: %s", setting_value);

    // Send a response, or redirect back to the main page
    httpd_resp_sendstr(req, "Updated successfully");

    return ESP_OK;
}

static const httpd_uri_t main = {
    .uri = "/", 
    .method = HTTP_GET, 
    .handler = send_temp_humidity_page, 
    .user_ctx = NULL
};

static const httpd_uri_t settings = {
    .uri = "/settings", 
    .method = HTTP_GET, 
    .handler = send_dropdown_page, 
    .user_ctx = NULL
};

static const httpd_uri_t update = {
    .uri = "/update",
    .method = HTTP_POST,
    .handler = handle_update_post,
    .user_ctx = NULL
};


static httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server ");
    ESP_LOGI(TAG, "TCP port: '%d'", config.server_port);
    ESP_LOGI(TAG, "UDP port: '%d'", config.ctrl_port);
    if (httpd_start(&server, &config) == ESP_OK) {

        //Set hostname
        // initialise_mdns();

        // Set URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &main);
        httpd_register_uri_handler(server, &settings);
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

static void disconnect_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server) {
        ESP_LOGI(TAG, "Stopping webserver");
        if (stop_webserver(*server) != ESP_OK) {
            ESP_LOGE(TAG, "Failed to stop http server");
        }
        *server = NULL;
    }
}

static void connect_handler(void* arg, esp_event_base_t event_base,
                            int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server == NULL) {
        ESP_LOGI(TAG, "Starting webserver");
        *server = start_webserver();
    }
}


esp_err_t http_server_start(void)
{
    // static httpd_handle_t server = NULL;

    /* Register event handlers to stop the server when Wi-Fi is disconnected,
     * and re-start it upon connection.
     */
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &connect_handler, &main_server));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disconnect_handler, &main_server));

    /* Start the server for the first time */
    main_server = start_webserver();

    return ESP_OK;
}
