#include "kernel.h"
#include "thread.h"
#include "memory.h"
#include "page_fault.h"
#include "channel.h"
#include "datetime.h"
#include "handler.h"
#include <resea.h>
#include <resea/memory.h>
#include <resea/channel.h>
#include <resea/thread.h>
#include <resea/datetime.h>
#include <resea/pager.h>
#include <resea/zeroed_pager.h>


namespace kernel {

// this startup() should be return
extern "C" void kernel_startup(void) {
    channel_t ch;

    INFO("starting Resea");
    INFO("built on " __DATE__);

    channel::init();
    memory::init();
    page_fault::init();
    thread::init();
    datetime::init();

    ch = create_channel();
    set_channel_handler(ch, kernel::server_handler);
    channel::register_server(ch, INTERFACE(thread));
    channel::register_server(ch, INTERFACE(memory));
    channel::register_server(ch, INTERFACE(datetime));
    channel::register_server(ch, INTERFACE(pager));
    channel::register_server(ch, INTERFACE(zeroed_pager));

    INFO("created a kernel server at @%d", ch);
}

} // namespace kernel
