#include "wifi.h"
#include <HTTPClient.h>
#include <esp_wifi.h>
#include "engine.h"
#include "smms.h"
#include "logger.h"
#include "BinaryStream.h"

static bool connected = false;

void connected_to_wifi() {
    connected = true;
}

WiFiSmmsClient::WiFiSmmsClient(Engine *engine, const char *wifi_ssid,
    const char *wifi_password, const char *server_url, const char *ca_cert,
    const char *device_id) :
    SmmsClient(engine, device_id), wifi_ssid(wifi_ssid),
    wifi_password(wifi_password), ca_cert(ca_cert) {

    String url = server_url;

    int schema_length = 0;
    if (url.startsWith("https://")) {
        schema_length = 8;
        port = 443;
        tls_enabled = true;
    } else if (url.startsWith("http://")) {
        schema_length = 7;
        port = 80;
        tls_enabled = false;
    } else {
        WARN("invalid url scheme, assuming HTTPS");
        schema_length = 0;
        port = 443;
        tls_enabled = true;
    }

    int port_sep_pos = url.indexOf(':', schema_length);
    hostname = url.substring(
        schema_length,
        (port_sep_pos > 0) ? port_sep_pos : url.length()
    );

    if (port_sep_pos > 0) {
        port = url.substring(port_sep_pos + 1).toInt();
    }

    wifi_init_config_t init_config = WIFI_INIT_CONFIG_DEFAULT();
    wifi_config_t config;
    memset(&config, 0, sizeof(config));
    strcpy((char *)config.sta.ssid, wifi_ssid);
    strcpy((char *)config.sta.password, wifi_password);

    INFO("Connecting to %s", wifi_ssid);
    esp_wifi_init(&init_config);
    esp_wifi_set_storage(WIFI_STORAGE_RAM);
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &config);
    esp_wifi_start();
    esp_wifi_connect();

    while (!connected) {
        vTaskDelay(100);
    }

    INFO("connected to Wi-Fi");
}


void WiFiSmmsClient::send_payload(const void *payload, size_t length) {
    HTTPClient http;
    String url;

    url += tls_enabled ? "https://" : "http://";
    url += hostname;
    url += ":";
    url += port;
    url += "/api/v1/smms";

    INFO("sending a smms payload...");
    if (tls_enabled) {
        http.begin(url, ca_cert);
    } else {
        http.begin(url);
    }

    http.addHeader("Connection", "close");

    DEBUG("POST %s", url.c_str());
    int code = http.POST((uint8_t *)payload, length);

    if (code > 0) {
        if (code == HTTP_CODE_OK) {
            BinaryStream *stream = new BinaryStream();
            http.writeToStream(stream);

            INFO("received a smms payload (%d bytes)", stream->length);
            receive_payload((const void *) stream->buffer, stream->length);
            delete stream;
        } else {
            INFO("server returned an error: %d", code);
        }
    } else {
        INFO("failed to connect: %s", http.errorToString(code).c_str());
    }

    http.end();
}

void WiFiSmmsClient::download_app(int version) {
    HTTPClient http;
    String url;

    url += tls_enabled ? "https://" : "http://";
    url += hostname;
    url += ":";
    url += port;
    url += "/api/v1/images/app/";
    url += device_id;
    url += "/";
    url += version;

    INFO("starting a request...");
    if (tls_enabled) {
        http.begin(url, ca_cert);
    } else {
        http.begin(url);
    }

    http.addHeader("Connection", "close");

    DEBUG("GET %s", url.c_str());
    int code = http.GET();

    if (code > 0) {
        if (code == HTTP_CODE_OK) {
            INFO("downloading app image...");
            // Assuming that String is binary-safe; that is, it allows NULL
            // character and invalid character sequences in its buffer.
            String image = http.getString();
            current_app_version = version;
            launch_app((const void *)image.c_str(), image.length());
        } else {
            INFO("server returned an error: %d\n", code);
        }
    } else {
        INFO("failed to connect: %s",
               http.errorToString(code).c_str());
    }

    http.end();
}
