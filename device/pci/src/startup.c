#include <resea.h>
#include <resea/channel.h>
#include <resea/pci.h>


handler_t pci_handler;

result_t pci_startup(void) {
    channel_t ch;
    result_t r;

    ch = sys_open();
    call_channel_register(connect_to_local(1), ch,
        INTERFACE(pci), &r);

    serve_channel(ch, &pci_handler);
}
