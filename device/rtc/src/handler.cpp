#include "rtc.h"
#include <resea.h>
#include <resea/cpp/memory.h>
#include <resea/rtc.h>
#include <resea/datetime_device.h>
#include "handler.h"


void rtc_handler(channel_t __ch, payload_t *m) {
    if ((m[0] & 1) != 1) {
        WARN("the first payload is not inline one (expected inline msgid_t)");
        return;
    }

    switch (EXTRACT_MSGID(m)) {
    case MSGID(datetime_device, get_date):
        DEBUG("received datetime_device.get_date");
        rtc_datetime_device_get_date(
            __ch
        );
        return;

#ifndef KERNEL
        // free readonly payloads sent via kernel (user-space)
#endif

    }

    WARN("unsupported message: msgid=%#x", EXTRACT_MSGID(m));
}
