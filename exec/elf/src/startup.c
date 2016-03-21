#include <resea.h>
#include <resea/exec.h>

handler_t elf_handler;

void elf_startup(void) {
    channel_t ch;

    INFO("starting");
    sys_open(&ch);
    register_channel(ch, INTERFACE(exec));
    serve_channel(ch, &elf_handler);
    INFO("started");
}
