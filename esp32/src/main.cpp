#include <driver/gpio.h>
#include <driver/uart.h>
#include <esp_event.h>
#include <esp_event_loop.h>
#include <esp_system.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <nvs_flash.h>
#include <sdkconfig.h>
#include <stdio.h>
#include <string.h>
#include "uart.h"
#include "wifi.h"
#include "logger.h"

esp_err_t system_event_callback(void *ctx, system_event_t *event) {
    if (event->event_id == SYSTEM_EVENT_STA_GOT_IP) {
        connected_to_wifi();
    }

    return ESP_OK;
}

const char *WIFI_SSID = "__REPLACE_ME_WIFI_SSID_abcdefghijklmnopqrstuvwxyz1234567890__";
const char *WIFI_PASSWORD = "__REPLACE_ME_WIFI_PASSWORD_abcdefghijklmnopqrstuvwxyz1234567890__";
const char *DEVICE_NAME = "__REPLACE_ME_DEVICE_NAME_abcdefghijklmnopqrstuvwxyz1234567890__";
const char *SERVER_URL = "__REPLACE_ME_SERVER_URL_abcdefghijklmnopqrstuvwxyz1234567890__";
const char *NETWORK_ADAPTER = "__REPLACE_ME_NETWORK_ADAPTER__";

TelemataClient *telemata = nullptr;

void uart_adapter_send_task(void *param) {
    while (1) {
        INFO("sending a packet...");
        telemata->send();
        vTaskDelay(3000 / portTICK_PERIOD_MS);
    }
}

void uart_adapter_task(void *param) {
    UartTelemataClient *uart_telemata = new UartTelemataClient();
    telemata = (TelemataClient *) uart_telemata;
    xTaskCreate(&uart_adapter_send_task, "uart_send", 16 * 1024, NULL, 5, NULL);

    uart_telemata->poll_uart();
}

void http_adapter_task(void *param) {
    telemata = new WiFiTelemataClient(WIFI_SSID, WIFI_PASSWORD, SERVER_URL, DEVICE_NAME);

    while (1) {
        telemata->send();
        vTaskDelay(3000 / portTICK_PERIOD_MS);
    }
}

void setup();
void loop();

void app_task(void *param) {
    setup();
    for (;;) {
        loop();
    }
}

extern "C" void app_main() {
    INFO("Initializing...");
    nvs_flash_init();
    tcpip_adapter_init();
    esp_event_loop_init(system_event_callback, NULL);

    INFO("Starting MakeStack...");

//    xTaskCreate(&uart_adapter_task, "uart_adapter", 16 * 1024, NULL, 5, NULL);
    xTaskCreate(&app_task, "app", 16 * 1024, NULL, 5, NULL);
    xTaskCreate(&http_adapter_task, "http_adapter", 16 * 1024, NULL, 5, NULL);
}
