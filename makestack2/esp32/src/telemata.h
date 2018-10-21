#pragma once
#include <Stream.h>
#include <stdarg.h>
#include <map>
#include <string>

enum TelemataClientState { no_app, running_app, update_app };

int parse_variable_length(uint8_t *buf, int buf_length, int *length);

#define TELEMATA_VERSION 1
#define TELEMATA_DEVICE_NAME_MSG  0x01
#define TELEMATA_LOG_MSG          0x02
#define TELEMATA_DEVICE_STATE_MSG 0x03
#define TELEMATA_UPDATE_MSG       0x0a
#define TELEMATA_COMMAND_MSG      0x0b
#define DEVICE_BOOTED 1
#define DEVICE_RUNNING 2
#define DEVICE_TYPE_ESP32 1

struct device_state_msg {
    uint8_t state;
    uint8_t type;
    uint8_t battery;
    uint8_t reserved;
    uint32_t version;
    uint32_t ram_free;
} __attribute__((packed));

class TelemataClient {
private:
    char *log_buffer;
    int log_length;
    int log_index;
    int log_allocated_length;
    std::map<std::string, void *> config;
    bool first_send;
    void (*command_callback)(const char *name, const char *arg);

    virtual void send_payload(const void *payload, size_t length) = 0;
    virtual void download_app(int version) = 0;

protected:
    const char *device_name;
    int receive_payload(const void *payload, size_t length);

public:
    TelemataClient(const char *device_name);

    void send();
    void append_log(char ch);
    void append_log(const char *str);
    void set_config(std::string &key, void *data);
    void set_command_callback(void (*callback)(const char *name, const char *arg));
};

extern TelemataClient *telemata;
