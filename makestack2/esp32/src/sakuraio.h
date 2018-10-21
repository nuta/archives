#pragma once
#include "telemata.h"

class SakuraioTelemataClient : public TelemataClient {
private:
    bool connected;
    bool downloading;
    uint8_t get_connection_status();
    int send_command(int cmd, uint8_t *data, int len, uint8_t *resp, uint8_t resp_len);
    void enqueue_tx(int ch, int type, void *buf, int len);
    void flush_tx_queue();
    uint8_t flush_rx_queue();
    int dequeue_rx(uint8_t *buf, int len);
    int8_t get_rx_queue_length();
    int request_file_download(int id);
    int get_file_size();
    int get_file_data(uint8_t *buf, int len);
    void print_download_status();
    bool is_downloading_file();
    uint64_t get_date_time();
    uint8_t get_signal_quality();
    const char *get_signal_quality_name();

public:
    SakuraioTelemataClient(const char *device_name);
    void connect();
    void receive_forever();
    void send_payload(const void *payload, size_t length);
    void download_app(int version);
};

void connected_to_wifi();
