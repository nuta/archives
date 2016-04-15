#include <resea.h>
#include <resea/x86.h>
#include <resea/hal.h>
#include "handler.h"


void x86_handler(channel_t __ch, payload_t *payloads) {
    if ((payloads[0] & 1) != 1) {
        WARN("the first payload is not inline one (expected inline msgtype_t)");
        return;
    }

    switch (payloads[1]) {
    }

    WARN("unsupported message: interface=%d, type=%d", payloads[2] >> 16, payloads[1] & 0xffff);
}
