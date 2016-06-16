#include "kernel.h"
#include <resea.h>
#include <resea/channel.h>
#include <resea/datetime_device.h>
#include "kernel.h"


static channel_t datetime_device_ch;

result_t kernel_get_date(uint32_t *year, uint32_t *date, uint32_t *nsec) {
    result_t r;

    /* XXX */
    if (!datetime_device_ch) {
        datetime_device_ch = create_channel();
        resea::interfaces::channel::call_connect(connect_to_local(1), datetime_device_ch,
            INTERFACE(datetime_device), &r);
    }

    resea::interfaces::datetime_device::call_get_date(datetime_device_ch, &r, year, date, nsec);
    return r;
}


void kernel_datetime_startup(void) {

    INFO("initializing the datetime system");
}

