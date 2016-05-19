#include <resea.h>
#include <resea/channel.h>
#include <resea/exec.h>

handler_t elf_handler;

void elf_startup(void) {
    channel_t ch;
    result_t r;

    INFO("starting");
    ch = create_channel();
    call_channel_register(connect_to_local(1), ch,
        INTERFACE(exec), &r);
    serve_channel(ch, elf_handler);
}
