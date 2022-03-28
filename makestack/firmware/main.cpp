#include <makestack/types.h>
#include <makestack/cred.h>
#include <makestack/logger.h>
#include <makestack/vm.h>
#include <makestack/port.h>
#include <makestack/serial_adapter.h>
#include <makestack/wifi_adapter.h>

void app_task() {
    run_app();
    vTaskDelete(NULL);
 }

void supervisor_main() {
    init_logger();
    printf("\n");

    INFO("[Makestack] Hello!");
    INFO("[Makestack] version=%llu", __cred.version);
    initArduino();

    if (!strcmp(__cred.adapter, "serial")) {
        xTaskCreate((TaskFunction_t) &serial_adapter_task, "serial_adapter_task", 8192 * 4, NULL, 10, NULL);
    }
    if (!strcmp(__cred.adapter, "wifi")) {
        start_wifi_adapter();
    }

    // FIXME: Wait for init_serial to finish initializing the serial port.
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    xTaskCreate((TaskFunction_t) &app_task, "app_task", 8192, NULL, 10, NULL);
}
