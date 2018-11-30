#include "arm.h"
#include <resea.h>
#include <resea/cpp/memory.h>
#include <resea/console_device.h>
#include <resea/arm.h>
#include <resea/hal.h>
#include "console_device_server.h"

namespace arm {

void server_handler(channel_t __ch, payload_t *m) {
    if ((m[0] & 1) != 1) {
        WARN("the first payload is not inline one (expected inline msgid_t)");
        return;
    }

    switch (EXTRACT_MSGID(m)) {
    case MSGID(console_device, read):
        DEBUG("received console_device.read");
        console_device_server::handle_read(
            __ch
        );
        return;

#ifndef KERNEL
        // free readonly payloads sent via kernel (user-space)
#endif

    case MSGID(console_device, write):
        DEBUG("received console_device.write");
        console_device_server::handle_write(
            __ch
            , (uint8_t) EXTRACT(m, console_device, write, data)
        );
        return;

#ifndef KERNEL
        // free readonly payloads sent via kernel (user-space)
#endif

    }

    WARN("unsupported message: msgid=%#x", EXTRACT_MSGID(m));
}

} // namespace arm