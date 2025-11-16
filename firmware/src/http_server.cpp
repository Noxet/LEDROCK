#include "http_server.h"

#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include "esp_netif.h"
#include "esp_eth.h"
#include "esp_timer.h"
#include "freertos/idf_additions.h"
#include "portmacro.h"
#include "protocol_examples_common.h"
#include "protocol_examples_utils.h"

#include <esp_http_server.h>
#include <vector>
#include <string>

#define MAX_CLIENT_FAILED 5

struct client
{
    int fd;
    int failedAttempts;
};

static httpd_handle_t server = NULL;
static std::vector<struct client> g_clients;

QueueHandle_t httpQueue = nullptr;
/* A simple example that demonstrates using websocket echo server
 */
static const char *TAG = "ws_echo_server";

/*
 * Structure holding server handle
 * and internal socket fd in order
 * to use out of request send
 */
struct async_resp_arg {
    httpd_handle_t hd;
    int fd;
};


/*
 * async send function, which we put into the httpd work queue
 */
static void ws_async_send(void *arg)
{
    static const char * data = "Async data";
    struct async_resp_arg *resp_arg = static_cast<struct async_resp_arg *>(arg);
    httpd_handle_t hd = resp_arg->hd;
    int fd = resp_arg->fd;
    httpd_ws_frame_t ws_pkt;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.payload = (uint8_t*)data;
    ws_pkt.len = strlen(data);
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;

    httpd_ws_send_frame_async(hd, fd, &ws_pkt);
    free(resp_arg);
}

static esp_err_t trigger_async_send(httpd_handle_t handle, httpd_req_t *req)
{
    struct async_resp_arg *resp_arg = static_cast<struct async_resp_arg *>(malloc(sizeof(struct async_resp_arg)));
    if (resp_arg == NULL) {
        return ESP_ERR_NO_MEM;
    }
    resp_arg->hd = req->handle;
    resp_arg->fd = httpd_req_to_sockfd(req);
    esp_err_t ret = httpd_queue_work(handle, ws_async_send, resp_arg);
    if (ret != ESP_OK) {
        free(resp_arg);
    }
    return ret;
}

static esp_err_t echo_handler(httpd_req_t *req)
{
    if (req->method == HTTP_GET)
    {
        ESP_LOGI(TAG, "Got GET req");
    }

    char *buf;
    size_t bufLen = httpd_req_get_hdr_value_len(req, "Host") + 1;
    if (bufLen > 1)
    {
        buf = (char *) malloc(bufLen);
        assert(buf);
        ESP_LOGI(TAG, "Allocated %zu bytes for header", bufLen);
        if (httpd_req_get_hdr_value_str(req, "Host", buf, bufLen) == ESP_OK)
        {
            ESP_LOGI(TAG, "Got header -> Host: %s", buf);
        }
        free(buf);
    }

    bufLen = httpd_req_get_url_query_len(req) + 1;
    char decParam[1024]{};
    if (bufLen > 0)
    {
        buf = (char *) malloc(bufLen);
        assert(buf);
        if (httpd_req_get_url_query_str(req, buf, bufLen) == ESP_OK)
        {
            char param[1024];

            if (httpd_query_key_value(buf, "msg", param, sizeof(param)) == ESP_OK)
            {
                ESP_LOGI(TAG, "Got query -> msg: %s", param);
                example_uri_decode(decParam, param, strnlen(param, 1024));
                ESP_LOGI(TAG, "Decoded -> msg: %s", decParam);
            }
        }
        free(buf);
    }

    httpd_resp_set_hdr(req, "Resp", "Ledrock");

    httpd_resp_send(req, decParam, HTTPD_RESP_USE_STRLEN);

    return ESP_OK;
}

/*
 * This handler echos back the received ws data
 * and triggers an async send if certain message received
 */
static esp_err_t ws_handler(httpd_req_t *req)
{
    if (req->method == HTTP_GET) {
        int client_fd = httpd_req_to_sockfd(req);
        g_clients.push_back({.fd = client_fd, .failedAttempts = 0});
        ESP_LOGI(TAG, "Handshake done, the new connection was opened");
        return ESP_OK;
    }
    ESP_LOGI(TAG, "request method: %d", req->method);
    httpd_ws_frame_t ws_pkt;
    uint8_t *buf = NULL;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;
    /* Set max_len = 0 to get the frame len */
    esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "httpd_ws_recv_frame failed to get frame len with %d", ret);
        return ret;
    }
    ESP_LOGI(TAG, "frame len is %d", ws_pkt.len);
    if (ws_pkt.len) {
        /* ws_pkt.len + 1 is for NULL termination as we are expecting a string */
        buf = static_cast<uint8_t *>(calloc(1, ws_pkt.len + 1));
        if (buf == NULL) {
            ESP_LOGE(TAG, "Failed to calloc memory for buf");
            return ESP_ERR_NO_MEM;
        }
        ws_pkt.payload = buf;
        /* Set max_len = ws_pkt.len to get the frame payload */
        ret = httpd_ws_recv_frame(req, &ws_pkt, ws_pkt.len);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "httpd_ws_recv_frame failed with %d", ret);
            free(buf);
            return ret;
        }
        ESP_LOGI(TAG, "Got packet with message: %s", ws_pkt.payload);
    }
    ESP_LOGI(TAG, "Packet type: %d", ws_pkt.type);
    if (ws_pkt.type == HTTPD_WS_TYPE_TEXT &&
        strcmp((char*)ws_pkt.payload,"Trigger async") == 0) {
        free(buf);
        return trigger_async_send(req->handle, req);
    }

    ret = httpd_ws_send_frame(req, &ws_pkt);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "httpd_ws_send_frame failed with %d", ret);
    }
    free(buf);
    return ret;
}

static const httpd_uri_t ws = {
        .uri        = "/ws",
        .method     = HTTP_GET,
        .handler    = ws_handler,
        .user_ctx   = NULL,
        .is_websocket = true
};

static const httpd_uri_t echo = {
    .uri = "/echo",
    .method = HTTP_GET,
    .handler = echo_handler,
    .user_ctx = nullptr,
    .is_websocket = false,
};


static httpd_handle_t start_webserver(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Registering the ws handler
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &ws);
        httpd_register_uri_handler(server, &echo);
        return server;
    }

    ESP_LOGI(TAG, "Error starting server!");
    return NULL;
}

static esp_err_t stop_webserver(httpd_handle_t server)
{
    // Stop the httpd server
    return httpd_stop(server);
}

static void disconnect_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server) {
        ESP_LOGI(TAG, "Stopping webserver");
        if (stop_webserver(*server) == ESP_OK) {
            *server = NULL;
        } else {
            ESP_LOGE(TAG, "Failed to stop http server");
        }
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


static void log_task(void *arg)
{
    (void) arg;

    char *msg = nullptr;
    while(1)
    {
        // Blocking call, wait for message to log
        xQueueReceive(httpQueue, &msg, portMAX_DELAY);
        httpd_ws_frame_t ws_pkt;
        memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
        ws_pkt.type = HTTPD_WS_TYPE_TEXT;
        ws_pkt.len = strlen(msg);
        ws_pkt.payload = (uint8_t *) msg;

        for (auto it = g_clients.begin(); it != g_clients.end();)
        {
            int ret = httpd_ws_send_frame_async(server, it->fd, &ws_pkt);
            if (ret != ESP_OK)
            {
                ESP_LOGI(TAG, "Failed to send log to client fd: %d", it->fd);
                it->failedAttempts++;
            }

            if (it->failedAttempts >= MAX_CLIENT_FAILED)
            {
                // Client probably disconnected, remove them from the list
                it = g_clients.erase(it);
            }
            else
            {
                ++it;
            }
        }
        vTaskDelay(500);
    }
}

void init(void)
{
    static httpd_handle_t server = NULL;
    httpQueue = xQueueCreate(20, sizeof(char *));

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());

    /* Register event handlers to stop the server when Wi-Fi or Ethernet is disconnected,
     * and re-start it upon connection.
     */
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &connect_handler, &server));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disconnect_handler, &server));


    /* Start the server for the first time */
    server = start_webserver();

    xTaskCreate(log_task, "ws_logs", 4096, nullptr, 5, nullptr);
}
