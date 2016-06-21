#include "kernel.h"
#include <resea.h>
#include <resea/kernel.h>
#include <resea/channel.h>
#include <resea/memory.h>
#include <resea/thread.h>
#include <resea/io.h>
#include <resea/datetime.h>
#include "datetime_server.h"


namespace kernel {
namespace datetime_server {

void handle_delay(channel_t __ch, uintmax_t msec) {

    resea::interfaces::datetime::send_delay_reply(__ch, OK);
}

void handle_get_date(channel_t __ch) {
    result_t r;
    uint32_t year, date, nsec;

    r = kernel_get_date(&year, &date, &nsec);
    resea::interfaces::datetime::send_get_date_reply(__ch, r, year, date, nsec);
}

void handle_set_interval_timer(channel_t __ch, channel_t ch, uintmax_t msec) {

}

void handle_set_oneshot_timer(channel_t __ch, channel_t ch, uintmax_t msec) {

}

} // namespace datetime_server
} // namespace kernel
