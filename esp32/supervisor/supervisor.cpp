#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "engine.h"
#include "smms.h"
#include "wifi.h"

const char *device_id = "i3Uu7cjwwY_6Ryv1B1y.GB~E~qRJ2iMRXF2RapjX";
const char *server_url = "http://172.18.0.105:8080";
Engine *app_engine = nullptr;
SmmsClient *smms = nullptr;

void supervisor_task(void *param) {
    smms = new WiFiSmmsClient(new Engine(), server_url, device_id);

    while (1) {
        smms->send();
        vTaskDelay(3000 / portTICK_PERIOD_MS);
    }
}
