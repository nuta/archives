#include <string.h>
#include <stdio.h>
#include <makestack.h>
#include "telemata.h"
#include "logger.h"

void publish(const char *event, char *value) {
    char buf[256];
    if (strlen(event) + strlen(value) + 3 > sizeof(buf)) {
        ERROR("too large publish");
        return;
    }

    sprintf((char *) &buf, "@%s %s", event, value);
    telemata->append_log((char *) &buf);
}
