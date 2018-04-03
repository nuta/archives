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

esp_err_t system_event_callback(void *ctx, system_event_t *event) {
    if (event->event_id == SYSTEM_EVENT_STA_GOT_IP) {
        connected_to_wifi();
    }

    return ESP_OK;
}

#define BLINK_GPIO ((gpio_num_t)12)
void blink_task(void *param) {
    gpio_pad_select_gpio(BLINK_GPIO);
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);

    while (1) {
        gpio_set_level(BLINK_GPIO, 0);
        vTaskDelay(300);
        gpio_set_level(BLINK_GPIO, 1);
        vTaskDelay(300);
    }
}

extern "C" void app_main() {
    printf("Initializing...\n");
    nvs_flash_init();
    tcpip_adapter_init();
    esp_event_loop_init(system_event_callback, NULL);
    init_wifi();

    xTaskCreate(&blink_task, "blink", 8192, NULL, 5, NULL);

    printf("Starting MakeStack...\n");
    xTaskCreate(&supervisor_task, "supervisor", 16 * 1024, NULL, 5, NULL);
}
