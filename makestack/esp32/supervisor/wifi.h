#pragma once
#include "smms.h"

class WiFiSmmsClient : public SmmsClient {
private:
    String hostname;
    const char *wifi_ssid;
    const char *wifi_password;
    const char *ca_cert;
    bool tls_enabled;
    int port;

public:
    WiFiSmmsClient(Engine *engine, const char *wifi_ssid,
        const char *wifi_password, const char *server_url, const char *ca_cert,
        const char *device_id);
    void send_payload(const void *payload, size_t length);
    void download_app(int version);
};

void connected_to_wifi();
