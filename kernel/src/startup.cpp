#include "kernel.h"
#include <resea.h>
#include <resea/memory.h>
#include <resea/channel.h>
#include <resea/thread.h>
#include <resea/datetime.h>
#include <resea/pager.h>
#include <resea/zeroed_pager.h>
#include "kernel.h"

static channel_t ch;

// this startup() should be return
extern "C" void kernel_startup(void) {
    INFO("starting Resea");
    INFO("built on " __DATE__);

    kernel_channel_startup();
    kernel_memory_startup();
    kernel_thread_startup();
    kernel_datetime_startup();

    ch = create_channel();
    set_channel_handler(ch, &kernel_handler);
    kernel_register_channel(ch, INTERFACE(thread));
    kernel_register_channel(ch, INTERFACE(memory));
    kernel_register_channel(ch, INTERFACE(datetime));
    kernel_register_channel(ch, INTERFACE(pager));
    kernel_register_channel(ch, INTERFACE(zeroed_pager));

    INFO("created a kernel server at @%d", ch);
}

