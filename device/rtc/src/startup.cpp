#include "rtc.h"
#include <resea.h>
#include <resea/channel.h>
#include <resea/rtc.h>
#include <resea/datetime_device.h>


handler_t rtc_handler;
bool rtc_initialized = false;

void rtc_startup(void) {
    channel_t ch;
    result_t r;

    ch = create_channel();
    call_channel_register(connect_to_local(1), ch,
        INTERFACE(datetime_device), &r);

    rtc_inialized = true;
    serve_channel(ch, &rtc_handler);
}
