#pragma once
#include <Stream.h>
#include <stdarg.h>
#include <map>
#include <string>
#include "engine.h"

enum SmmsClientState { no_app, running_app, update_app };

class SmmsClient {
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
    Engine *engine;
    const char *device_id;
    void receive_payload(const void *payload, size_t length);

   public:
    SmmsClient(Engine *engine, const char *device_id);

    void send();
    void append_log(char ch);
    void append_log(const char *str);
    void set_config(std::string &key, void *data);
    void update_app(int version);
    void launch_app(const void *image, size_t image_length);
};

extern SmmsClient *smms;
