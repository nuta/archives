#pragma once
#include "telemata.h"

class UartTelemataClient : public TelemataClient {
private:

public:
    UartTelemataClient();
    void poll_uart();
    void send_payload(const void *payload, size_t length);
    void download_app(int version);
};
