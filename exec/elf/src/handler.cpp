#include <resea.h>
#include <resea/elf.h>
#include <resea/exec.h>
#include "handler.h"


void elf_handler(channel_t __ch, payload_t *payloads) {
    if ((payloads[0] & 1) != 1) {
        WARN("the first payload is not inline one (expected inline msgid_t)");
        return;
    }

    switch (payloads[1]) {
    case MSGID(exec, create):
    {
        DEBUG("received exec.create");
            payload_t a0 = payloads[2];
            payload_t a1 = payloads[3];
            payload_t a2 = payloads[4];
            payload_t a3 = payloads[5];
            payload_t a4 = payloads[6];
            payload_t a5 = payloads[7];
            elf_exec_create(__ch, (uchar_t*) a0, (size_t) a1, (size_t) a2, (channel_t) a3, (ident_t) a4, (ident_t) a5);
            return;
    }
    }

    WARN("unsupported message: interface=%d, type=%d", payloads[2] >> 16, payloads[1] & 0xffff);
}
