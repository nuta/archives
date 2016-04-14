#include <resea.h>
#include <resea/memory.h>
#include <resea/channel.h>
#include <resea/thread.h>
#include <resea/datetime.h>
#include "kernel.h"

static channel_t ch;

// this startup() should be return
void kernel_startup(void) {
    INFO("starting Resea");
    INFO("built on " __DATE__);

    kernel_channel_startup();
    kernel_thread_startup();
    kernel_memory_startup();
    kernel_datetime_startup(); 

    ch = sys_open();
    sys_setoptions(ch, &kernel_handler, NULL, 0);
    register_channel(ch, INTERFACE(thread));
    register_channel(ch, INTERFACE(memory));
    register_channel(ch, INTERFACE(datetime));
    register_channel(ch, INTERFACE(channel));
    INFO("created a kernel server at @%d", ch);

    hal_create_vm_space(&kernel_get_current_thread_group()->vm); /* XXX */
}

