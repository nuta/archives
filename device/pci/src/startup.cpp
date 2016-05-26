#include "pci.h"
#include <resea.h>
#include <resea/channel.h>
#include <resea/pci.h>


handler_t pci_handler;
bool pci_initialized = false;

result_t pci_startup(void) {
    channel_t ch;
    result_t r;

    ch = create_channel();
    call_channel_register(connect_to_local(1), ch,
        INTERFACE(pci), &r);

    pci_initialized = true;
    serve_channel(ch, &pci_handler);
}
