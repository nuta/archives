#pragma once
#include "smms.h"

class WiFiSmmsClient : public SmmsClient {
   public:
    WiFiSmmsClient(Engine *engine, const char *device_id)
        : SmmsClient(engine, device_id) {}
    void send_payload(const void *payload, size_t length);
    void download_app(int version);
};

void connected_to_wifi();
void init_wifi();
