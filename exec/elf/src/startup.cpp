#include "elf.h"
#include "handler.h"
#include <resea.h>
#include <resea/channel.h>
#include <resea/exec.h>


extern "C" void elf_startup(void) {
    channel_t ch;
    result_t r;

    INFO("starting");
    ch = create_channel();
    resea::interfaces::channel::call_register(connect_to_local(1), ch,
        INTERFACE(exec), &r);
    serve_channel(ch, elf::server_handler);
}
