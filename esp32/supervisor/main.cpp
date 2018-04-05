#include <driver/gpio.h>
#include <esp_event.h>
#include <esp_event_loop.h>
#include <esp_system.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <nvs_flash.h>
#include <sdkconfig.h>
#include <stdio.h>
#include <string.h>
#include "supervisor.h"
#include "wifi.h"
#include "logger.h"

esp_err_t system_event_callback(void *ctx, system_event_t *event) {
    if (event->event_id == SYSTEM_EVENT_STA_GOT_IP) {
        connected_to_wifi();
    }

    return ESP_OK;
}

extern "C" void app_main() {
    INFO("Initializing...");
    nvs_flash_init();
    tcpip_adapter_init();
    esp_event_loop_init(system_event_callback, NULL);
    init_wifi();

    INFO("Starting MakeStack...");
    xTaskCreate(&supervisor_task, "supervisor", 16 * 1024, NULL, 5, NULL);
}
