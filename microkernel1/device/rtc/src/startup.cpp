#include "rtc.h"
#include "handler.h"
#include <resea.h>
#include <resea/channel.h>
#include <resea/rtc.h>
#include <resea/datetime_device.h>


extern "C" void rtc_startup(void) {
    channel_t ch;
    result_t r;

    ch = create_channel();
    resea::interfaces::channel::call_register(connect_to_local(1), ch,
        INTERFACE(datetime_device), &r);

    serve_channel(ch, rtc::server_handler);
}
