#include "rtc.h"
#include <resea.h>
#include <resea/datetime_device.h>
#include <resea/rtc.h>

namespace rtc {
void server_handler(channel_t __ch, payload_t *m);

namespace datetime_device_server {
void handle_get_date(
    channel_t __ch
);
} // namespace datetime_device

} // namespace rtc
