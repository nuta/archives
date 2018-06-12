#include <string.h>
#include <stdio.h>
#include <makestack.h>
#include "telemata.h"
#include "logger.h"

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
    va_end(vargs);
}


void publish(const char *event, char *value) {
    char buf[256];
    if (strlen(event) + strlen(value) + 3 > sizeof(buf)) {
        ERROR("too large publish");
        return;
    }

    sprintf((char *) &buf, "@%s %s", event, value);
    telemata->append_log((char *) &buf);
}
