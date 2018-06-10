#include "wifi.h"
#include <HTTPClient.h>
#include <esp_wifi.h>
#include <esp_ota_ops.h>
#include "telemata.h"
#include "logger.h"
#include "BinaryStream.h"

static bool connected = false;

void connected_to_wifi() {
    connected = true;
}

WiFiTelemataClient::WiFiTelemataClient(const char *wifi_ssid,
    const char *wifi_password, const char *server_url, const char *ca_cert,
    const char *device_id) :
    TelemataClient(device_id), wifi_ssid(wifi_ssid),
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


void WiFiTelemataClient::send_payload(const void *payload, size_t length) {
    HTTPClient http;
    String url;

    url += tls_enabled ? "https://" : "http://";
    url += hostname;
    url += ":";
    url += port;
    url += "/makestack/telemata";

    INFO("sending a telemata payload...");
    if (tls_enabled) {
        ca_cert = "-----BEGIN CERTIFICATE-----\n"
"MIIEXDCCA0SgAwIBAgINAeOpMBz8cgY4P5pTHTANBgkqhkiG9w0BAQsFADBMMSAw\n"
"HgYDVQQLExdHbG9iYWxTaWduIFJvb3QgQ0EgLSBSMjETMBEGA1UEChMKR2xvYmFs\n"
"U2lnbjETMBEGA1UEAxMKR2xvYmFsU2lnbjAeFw0xNzA2MTUwMDAwNDJaFw0yMTEy\n"
"MTUwMDAwNDJaMFQxCzAJBgNVBAYTAlVTMR4wHAYDVQQKExVHb29nbGUgVHJ1c3Qg\n"
"U2VydmljZXMxJTAjBgNVBAMTHEdvb2dsZSBJbnRlcm5ldCBBdXRob3JpdHkgRzMw\n"
"ggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDKUkvqHv/OJGuo2nIYaNVW\n"
"XQ5IWi01CXZaz6TIHLGp/lOJ+600/4hbn7vn6AAB3DVzdQOts7G5pH0rJnnOFUAK\n"
"71G4nzKMfHCGUksW/mona+Y2emJQ2N+aicwJKetPKRSIgAuPOB6Aahh8Hb2XO3h9\n"
"RUk2T0HNouB2VzxoMXlkyW7XUR5mw6JkLHnA52XDVoRTWkNty5oCINLvGmnRsJ1z\n"
"ouAqYGVQMc/7sy+/EYhALrVJEA8KbtyX+r8snwU5C1hUrwaW6MWOARa8qBpNQcWT\n"
"kaIeoYvy/sGIJEmjR0vFEwHdp1cSaWIr6/4g72n7OqXwfinu7ZYW97EfoOSQJeAz\n"
"AgMBAAGjggEzMIIBLzAOBgNVHQ8BAf8EBAMCAYYwHQYDVR0lBBYwFAYIKwYBBQUH\n"
"AwEGCCsGAQUFBwMCMBIGA1UdEwEB/wQIMAYBAf8CAQAwHQYDVR0OBBYEFHfCuFCa\n"
"Z3Z2sS3ChtCDoH6mfrpLMB8GA1UdIwQYMBaAFJviB1dnHB7AagbeWbSaLd/cGYYu\n"
"MDUGCCsGAQUFBwEBBCkwJzAlBggrBgEFBQcwAYYZaHR0cDovL29jc3AucGtpLmdv\n"
"b2cvZ3NyMjAyBgNVHR8EKzApMCegJaAjhiFodHRwOi8vY3JsLnBraS5nb29nL2dz\n"
"cjIvZ3NyMi5jcmwwPwYDVR0gBDgwNjA0BgZngQwBAgIwKjAoBggrBgEFBQcCARYc\n"
"aHR0cHM6Ly9wa2kuZ29vZy9yZXBvc2l0b3J5LzANBgkqhkiG9w0BAQsFAAOCAQEA\n"
"HLeJluRT7bvs26gyAZ8so81trUISd7O45skDUmAge1cnxhG1P2cNmSxbWsoiCt2e\n"
"ux9LSD+PAj2LIYRFHW31/6xoic1k4tbWXkDCjir37xTTNqRAMPUyFRWSdvt+nlPq\n"
"wnb8Oa2I/maSJukcxDjNSfpDh/Bd1lZNgdd/8cLdsE3+wypufJ9uXO1iQpnh9zbu\n"
"FIwsIONGl1p3A8CgxkqI/UAih3JaGOqcpcdaCIzkBaR9uYQ1X4k2Vg5APRLouzVy\n"
"7a8IVk6wuy6pm+T7HT4LY8ibS5FEZlfAFLSW8NwsVz9SBK2Vqn1N0PIMn5xA6NZV\n"
"c7o835DLAFshEWfC7TIe3g==\n"
"-----END CERTIFICATE-----";
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

            INFO("received a telemata payload (%d bytes)", stream->length);
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

struct firmware_header {
    uint8_t magic[4];
    uint32_t length;
    uint32_t version;
    uint32_t reserved;
} __attribute__((packed));

void update_firmware(WiFiClient *client) {
    if (!client) {
        DEBUG("not connected\n");
        return;
    }

    esp_ota_handle_t update_handle;
    const esp_partition_t *part;
    part = esp_ota_get_next_update_partition(NULL);
    if (part == NULL) {
        printf("%s: failed to get next partition\n", __func__);
        return;
    }

    esp_err_t err = esp_ota_begin(part, OTA_SIZE_UNKNOWN, &update_handle);
    if (err != ESP_OK) {
        printf("esp_ota_begin: %s\n", esp_err_to_name(err));
        return;
    }

    struct firmware_header header;
    int len = client->read((uint8_t *) &header, sizeof(header));
    if (len < sizeof(header)) {
        return;
    }

    printf("recievd: %x %s\n", header.magic[0], (char *) &header);
    if (header.magic[0] != 0x81 || header.magic[1] != 0xf1
        || header.magic[2] != 0x5c || header.magic[3] != 0x5e) {
        printf("invalid firmware signature\n");
        return;
    }


    int buf_size = 8192;
    uint8_t *buf = (uint8_t *) malloc(buf_size);
    int offset = 0;
    while ((client->connected() || client->available()) && offset < header.length) {
        int read_size = (header.length - offset < buf_size) ? header.length - offset : buf_size;
        int len = client->read(buf, read_size);
        if (len < 0) {
            continue;
        }

        printf("Downloading %d/%d KB\n", offset / 1024, header.length / 1024);

        if ((err = esp_ota_write(update_handle, buf, len)) != ESP_OK) {
            printf("esp_ota_write: %s\n", esp_err_to_name(err));
            free(buf);
            return;
        }

        offset += len;
    }

    free(buf);

    if (offset < header.length) {
        printf("disconnected\n");
        return;
    }

    if (esp_ota_end(update_handle) != ESP_OK) {
        printf("esp_ota_end: %s\n", esp_err_to_name(err));
        return;
    }

    if ((err = esp_ota_set_boot_partition(part)) != ESP_OK) {
        printf("esp_ota_set_boot_partition: %s\n", esp_err_to_name(err));
        return;
    }

    printf("restarting the system...\n");
    esp_restart();
    printf("failed to restart :(\n");
}


void WiFiTelemataClient::download_app(int version) {
    HTTPClient http;
    String url;

    url += tls_enabled ? "https://" : "http://";
    url += hostname;
    url += ":";
    url += port;
    url += "/makestack/firmware";

    INFO("starting a request...");
    if (tls_enabled) {
        http.begin(url, ca_cert);
    } else {
        http.begin(url);
    }

    http.addHeader("Connection", "close");

    DEBUG("GET %s", url.c_str());
    int code = http.GET();

    if (code <= 0) {
        INFO("failed to connect: %s",
             http.errorToString(code).c_str());
        goto error;
    }

    if (code != HTTP_CODE_OK) {
        INFO("server returned an error: %d\n", code);
        goto error;
    }

    update_firmware(http.getStreamPtr());

error:
    http.end();
}
