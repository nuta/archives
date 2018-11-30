#include "arm.h"
#include <resea.h>
#include <resea/console_device.h>
#include <resea/arm.h>
#include <resea/hal.h>

namespace arm {
namespace console_device_server {

void handle_read(
    channel_t __ch
);
void handle_write(
    channel_t __ch
    , uint8_t data
);

} // namespace console_device
} // namespace arm
