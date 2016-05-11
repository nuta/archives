#include <resea.h>
#include <resea/rtc.h>
#include <resea/datetime_device.h>
#include "handler.h"


void rtc_handler(channel_t __ch, payload_t *payloads) {
    if ((payloads[0] & 1) != 1) {
        WARN("the first payload is not inline one (expected inline msgid_t)");
        return;
    }

    switch (payloads[1]) {
    case MSGID(datetime_device, get_date):
    {
        DEBUG("received datetime_device.get_date");
            rtc_datetime_device_get_date(__ch);
            return;
    }
    }

    WARN("unsupported message: interface=%d, type=%d", payloads[2] >> 16, payloads[1] & 0xffff);
}
