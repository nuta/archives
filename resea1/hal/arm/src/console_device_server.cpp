#include "arm.h"
#include <resea.h>
#include <resea/console_device.h>
#include <resea/arm.h>
#include <resea/hal.h>
#include "console_device_server.h"

using namespace resea::interfaces;

namespace arm {
namespace console_device_server {


void handle_read(channel_t __ch) {
    uint8_t data;
    result_t r;

    r = console_read(&data);
    console_device::send_read_reply(__ch, r, data);
}


void handle_write(channel_t __ch, uint8_t data) {
    result_t r;

    r = console_write(data);
    console_device::send_write_reply(__ch, r);
}


} // namespace console_device_server
} // namespace arm
