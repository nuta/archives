#pragma once
#include "telemata.h"

class WiFiTelemataClient : public TelemataClient {
private:
    String hostname;
    const char *wifi_ssid;
    const char *wifi_password;
    bool tls_enabled;
    int port;

public:
    WiFiTelemataClient(const char *wifi_ssid,
        const char *wifi_password, const char *server_url, const char *device_name);
    void send_payload(const void *payload, size_t length);
    void download_app(int version);
};

void connected_to_wifi();
