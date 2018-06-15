#include <map>
#include <string.h>
#include <stdio.h>
#include <makestack.h>
#include "telemata.h"
#include "logger.h"
#include "api.h"

void print(const char *fmt, ...) {
    char buf[256];
    va_list vargs;
    va_start(vargs, fmt);
    vsnprintf((char *) &buf, sizeof(buf), fmt, vargs);
    telemata->append_log((char *) &buf);
    va_end(vargs);
}


void println(const char *fmt, ...) {
    char buf[256];
    va_list vargs;
    va_start(vargs, fmt);
    vsnprintf((char *) &buf, sizeof(buf), fmt, vargs);
    telemata->append_log((char *) &buf);
    telemata->append_log('\n');
    printf("%s\n", (char *) &buf);
    va_end(vargs);
}


void publish(const char *event, char *value) {
    char buf[256];
    if (strlen(event) + strlen(value) + 3 > sizeof(buf)) {
        ERROR("too large publish");
        return;
    }

    sprintf((char *) &buf, "@%s %s", event, value);
    printf("%s\n", (char *) &buf);
    telemata->append_log((char *) &buf);
}


static std::map<std::string, command_handler_t> *handlers;

void command(const char *name, command_handler_t handler) {
    handlers->emplace(std::string(name), handler);
}

void command_callback(const char *name, const char *arg) {
    auto it = handlers->find(std::string(name));
    if (it != handlers->end()) {
        command_handler_t handler = it->second;
        handler(String(arg));
    }
}

void init_api() {
    handlers = new std::map<std::string, command_handler_t>();
    telemata->set_command_callback(command_callback);
}
