#include <resea.h>
#include <resea/channel.h>
#include <resea/rtc.h>
#include <resea/datetime_device.h>


handler_t rtc_handler;

extern "C" void rtc_startup(void) {
    channel_t ch;
    result_t r;

    ch = create_channel();
    call_channel_register(connect_to_local(1), ch,
        INTERFACE(datetime_device), &r);

    serve_channel(ch, &rtc_handler);
}
