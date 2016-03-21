#include <resea.h>
#include <resea/rtc.h>
#include <resea/datetime_device.h>


handler_t rtc_handler;

result_t rtc_startup(void) {
    channel_t ch;

    sys_open(&ch);
    register_channel(ch, INTERFACE(datetime_device));
    serve_channel(ch, &rtc_handler);
}
