#pragma once
#include "smms.h"

class WiFiSmmsClient : public SmmsClient {
private:
    String hostname;
    bool tls_enabled;
    int port;

public:
    WiFiSmmsClient(Engine *engine, const char *server_url, const char *device_id);
    void send_payload(const void *payload, size_t length);
    void download_app(int version);
};

void connected_to_wifi();
void wifi_init();
