#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "engine.h"
#include "smms.h"
#include "wifi.h"

const char *ca_cert = "_____REPLACE_ME_MAKESTACK_CONFIG_CA_CERT_____";
const char *device_id = "_____REPLACE_ME_MAKESTACK_CONFIG_DEVICE_ID_____";
const char *device_secret = "_____REPLACE_ME_MAKESTACK_CONFIG_DEVICE_SECRET_____";
const char *device_type = "_____REPLACE_ME_MAKESTACK_CONFIG_DEVICE_TYPE_____";
const char *server_url = "_____REPLACE_ME_MAKESTACK_CONFIG_SERVER_URL_abcdefghijklmnopqrstuvwxyz1234567890_____";
const char *network_adapter = "_____REPLACE_ME_MAKESTACK_CONFIG_NETWORK_ADAPTER_____";
const char *wifi_ssid = "_____REPLACE_ME_MAKESTACK_CONFIG_WIFI_SSID_____";
const char *wifi_password = "_____REPLACE_ME_MAKESTACK_CONFIG_WIFI_PSK_____";
const char *wifi_country = "_____REPLACE_ME_MAKESTACK_CONFIG_WIFI_COUNTRY_____";
Engine *app_engine = nullptr;
SmmsClient *smms = nullptr;

void supervisor_task(void *param) {
    smms = new WiFiSmmsClient(new Engine(), wifi_ssid, wifi_password, server_url,
                              ca_cert, device_id);

    while (1) {
        smms->send();
        vTaskDelay(3000 / portTICK_PERIOD_MS);
    }
}
