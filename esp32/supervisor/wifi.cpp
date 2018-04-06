#include "wifi.h"
#include <HTTPClient.h>
#include <esp_wifi.h>
#include "engine.h"
#include "smms.h"
#include "logger.h"
#include "BinaryStream.h"

#define WIFI_SSID "curve25519"
#define WIFI_PASSWORD "howudoing"

// COMODO ECC Certification Authority (used for Cloudflare SNI ones)
const char *root_ca_cerificates =
    "-----BEGIN CERTIFICATE-----\n"
    "MIID0DCCArigAwIBAgIQQ1ICP/qokB8Tn+P05cFETjANBgkqhkiG9w0BAQwFADBv\n"
    "MQswCQYDVQQGEwJTRTEUMBIGA1UEChMLQWRkVHJ1c3QgQUIxJjAkBgNVBAsTHUFk\n"
    "ZFRydXN0IEV4dGVybmFsIFRUUCBOZXR3b3JrMSIwIAYDVQQDExlBZGRUcnVzdCBF\n"
    "eHRlcm5hbCBDQSBSb290MB4XDTAwMDUzMDEwNDgzOFoXDTIwMDUzMDEwNDgzOFow\n"
    "gYUxCzAJBgNVBAYTAkdCMRswGQYDVQQIExJHcmVhdGVyIE1hbmNoZXN0ZXIxEDAO\n"
    "BgNVBAcTB1NhbGZvcmQxGjAYBgNVBAoTEUNPTU9ETyBDQSBMaW1pdGVkMSswKQYD\n"
    "VQQDEyJDT01PRE8gRUNDIENlcnRpZmljYXRpb24gQXV0aG9yaXR5MHYwEAYHKoZI\n"
    "zj0CAQYFK4EEACIDYgAEA0d7L3XJghWF+3XkkRbUq2KZ9T5SCwbOQQB/l+EKJDwd\n"
    "AQTuPdKNCZcM4HXk+vt3iir1A2BLNosWIxatCXH0SvQoULT+iBxuP2wvLwlZW6Vb\n"
    "CzOZ4sM9iflqLO+y0wbpo4H+MIH7MB8GA1UdIwQYMBaAFK29mHo0tCb3+sQmVO8D\n"
    "veAky1QaMB0GA1UdDgQWBBR1cacZSBm8nZ3qQUfflMRId5nTeTAOBgNVHQ8BAf8E\n"
    "BAMCAYYwDwYDVR0TAQH/BAUwAwEB/zARBgNVHSAECjAIMAYGBFUdIAAwSQYDVR0f\n"
    "BEIwQDA+oDygOoY4aHR0cDovL2NybC50cnVzdC1wcm92aWRlci5jb20vQWRkVHJ1\n"
    "c3RFeHRlcm5hbENBUm9vdC5jcmwwOgYIKwYBBQUHAQEELjAsMCoGCCsGAQUFBzAB\n"
    "hh5odHRwOi8vb2NzcC50cnVzdC1wcm92aWRlci5jb20wDQYJKoZIhvcNAQEMBQAD\n"
    "ggEBAB3H+i5AtlwFSw+8VTYBWOBTBT1k+6zZpTi4pyE7r5VbvkjI00PUIWxB7Qkt\n"
    "nHMAcZyuIXN+/46NuY5YkI78jG12yAA6nyCmLX3MF/3NmJYyCRrJZfwE67SaCnjl\n"
    "lztSjxLCdJcBns/hbWjYk7mcJPuWJ0gBnOqUP3CYQbNzUTcp6PYBerknuCRR2RFo\n"
    "1KaFpzanpZa6gPim/a5thCCuNXZzQg+HCezF3OeTAyIal+6ailFhp5cmHunudVEI\n"
    "kAWvL54TnJM/ev/m6+loeYyv4Lb67psSE/5FjNJ80zXrIRKT/mZ1JioVhCb3ZsnL\n"
    "jbsJQdQYr7GzEPUQyp2aDrV1aug=\n"
    "-----END CERTIFICATE-----\n";


WiFiSmmsClient::WiFiSmmsClient(Engine *engine, const char *server_url, const char *device_id)
    : SmmsClient(engine, device_id) {

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
        http.begin(url, root_ca_cerificates);
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
        http.begin(url, root_ca_cerificates);
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

static bool connected = false;

void connected_to_wifi() {
    connected = true;
}

void wifi_init() {
    wifi_init_config_t init_config = WIFI_INIT_CONFIG_DEFAULT();
    wifi_config_t config;
    memset(&config, 0, sizeof(config));
    strcpy((char *)config.sta.ssid, WIFI_SSID);
    strcpy((char *)config.sta.password, WIFI_PASSWORD);

    INFO("Connecting to %s", WIFI_SSID);
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
