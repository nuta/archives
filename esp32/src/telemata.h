#pragma once
#include <Stream.h>
#include <stdarg.h>
#include <map>
#include <string>

enum TelemataClientState { no_app, running_app, update_app };

int parse_variable_length(uint8_t *buf, int buf_length, int *length);

class TelemataClient {
   private:
    int state;
    char *log_buffer;
    int log_length;
    int log_index;
    int log_allocated_length;
    std::map<std::string, void *> config;

    virtual void send_payload(const void *payload, size_t length) = 0;
    virtual void download_app(int version) = 0;

   protected:
    unsigned long current_app_version;
    const char *device_name;
    int receive_payload(const void *payload, size_t length);

   public:
    TelemataClient(const char *device_name);

    void send();
    void append_log(char ch);
    void append_log(const char *str);
    void set_config(std::string &key, void *data);
};

extern TelemataClient *telemata;
