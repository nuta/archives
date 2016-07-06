#include "kernel.h"
#include "datetime.h"
#include <resea.h>
#include <resea/channel.h>
#include <resea/datetime_device.h>


using namespace resea::interfaces;

namespace kernel {
namespace datetime {


namespace {
    channel_t datetime_device_server;
}


result_t get_date(uint32_t *year, uint32_t *date, uint32_t *nsec) {
    result_t r;

    /* XXX */
    if (!datetime_device_server) {
        datetime_device_server = create_channel();
        channel::call_connect(connect_to_local(1), datetime_device_server,
            INTERFACE(datetime_device), &r);
    }

    datetime_device::call_get_date(datetime_device_server, &r, year, date, nsec);
    return r;
}


void init() {

    INFO("initializing the datetime system");
}


} // namespace datetime
} // namespace kernel
