#include "arm.h"
#include "handler.h"
#include <resea.h>
#include <stdarg.h>
#include <resea/channel.h>
#include <resea/console_device.h>

void start_apps(void);
extern "C" void kernel_startup(void);
void hal_printchar(char);

using namespace resea::interfaces;

namespace {
  channel_t server_ch;
}

extern "C" void arm_startup(void) {

    kernel_startup();

    result_t r;
    channel_t ch = create_channel();
    set_channel_handler(ch, arm::server_handler);
    channel::call_register(connect_to_local(1), ch,
        INTERFACE(console_device), &r);

    start_apps();
}
