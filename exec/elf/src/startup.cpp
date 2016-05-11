#include <resea.h>
#include <resea/exec.h>

handler_t elf_handler;

void elf_startup(void) {
    channel_t ch;

    INFO("starting");
    create_channel(&ch);
    register_channel(ch, INTERFACE(exec));
    serve_channel(ch, &elf_handler);
    INFO("started");
}
