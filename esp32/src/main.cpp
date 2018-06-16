#include <driver/gpio.h>
#include <driver/uart.h>
#include <esp_event.h>
#include <esp_event_loop.h>
#include <esp_system.h>
#include <esp_spi_flash.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <nvs_flash.h>
#include <sdkconfig.h>
#include <stdio.h>
#include <string.h>
#include <makestack.h>
#include "uart.h"
#include "wifi.h"
#include "sakuraio.h"
#include "logger.h"
#include "api.h"

esp_err_t system_event_callback(void *ctx, system_event_t *event) {
    if (event->event_id == SYSTEM_EVENT_STA_GOT_IP) {
        connected_to_wifi();
    }

    return ESP_OK;
}

#define CREDENTIALS_SIZE 1024
struct credentials_struct {
    char device_name[64];
    char server_url[256];
    char network_adapter[32];
    char wifi_ssid[64];
    char wifi_password[64];
};

struct credentials_struct *credentials = nullptr;
WiFiTelemataClient *wifi_telemata = nullptr;
UartTelemataClient *uart_telemata = nullptr;
SakuraioTelemataClient *sakuraio_telemata = nullptr;
TelemataClient *telemata = nullptr;

void uart_adapter_send_task(void *param) {
    while (1) {
        INFO("sending a packet...");
        telemata->send();
        vTaskDelay(3000 / portTICK_PERIOD_MS);
    }
}

void uart_adapter_task(void *param) {
    uart_telemata->poll_uart();
}

void wifi_adapter_heartbeat_task(void *param) {
    while (1) {
        wifi_telemata->send();
        vTaskDelay(3000 / portTICK_PERIOD_MS);
    }
}

void sakuraio_adapter_send_task(void *param) {
    while (1) {
        sakuraio_telemata->send();
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}

void sakuraio_adapter_recv_task(void *param) {
    sakuraio_telemata->receive_forever();
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

    INFO("MakeStack version " FIRMWARE_VERSION " (%s: %d)",
#ifdef RELEASE_BUILD
        "production"
#else
        "development"
#endif
        , APP_VERSION);

    credentials = (struct credentials_struct *) malloc(CREDENTIALS_SIZE);
    spi_flash_read(0x291000, credentials, CREDENTIALS_SIZE);

    if (strcmp(credentials->network_adapter, "wifi") == 0) {
        wifi_telemata = new WiFiTelemataClient(
            credentials->wifi_ssid, credentials->wifi_password,
            credentials->server_url, credentials->device_name
        );
        telemata = (TelemataClient *) wifi_telemata;

        // xTaskCreate(&wifi_adapter_heartbeat_task, "heartbeat", 16 * 1024, NULL, 5, NULL);
    } else if (strcmp(credentials->network_adapter, "serial") == 0) {
        uart_telemata = new UartTelemataClient();
        telemata = (TelemataClient *) uart_telemata;
        xTaskCreate(&uart_adapter_send_task, "uart_send", 16 * 1024, NULL, 5, NULL);
    } else if (strcmp(credentials->network_adapter, "sakuraio") == 0) {
        sakuraio_telemata = new SakuraioTelemataClient(credentials->device_name);
        telemata = (TelemataClient *) sakuraio_telemata;
        xTaskCreate(&sakuraio_adapter_send_task, "sakura_send", 16 * 1024, NULL, 5, NULL);
        xTaskCreate(&sakuraio_adapter_recv_task, "sakura_recv", 16 * 1024, NULL, 5, NULL);
    }

    init_api();

    if (strcmp(credentials->network_adapter, "sakuraio") == 0) {
        // sakuraio adapter depends on Wire API, initialized in init_api().
        sakuraio_telemata->connect();
    }

    xTaskCreate(&app_task, "app", 16 * 1024, NULL, 5, NULL);
}
