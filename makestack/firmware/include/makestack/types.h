#ifndef __MAKESTACK_TYPES_H__
#define __MAKESTACK_TYPES_H__

// TODO: Reduce #include
#include <driver/gpio.h>
#include <driver/uart.h>
#include <esp_event.h>
#include <esp_event_loop.h>
#include <esp_system.h>
#include <esp_spi_flash.h>
#include <esp_ota_ops.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <nvs_flash.h>
#include <sdkconfig.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#ifndef MAKESTACK_HEARTBEAT_INTERVAL
#define MAKESTACK_HEARTBEAT_INTERVAL 15
#endif

template<typename T>
static inline T min(T a, T b) {
    return (a < b) ? a : b;
}

template<typename T>
static inline T max(T a, T b) {
    return (a > b) ? a : b;
}

#endif
