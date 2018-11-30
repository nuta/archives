#include "pci.h"
#include "handler.h"
#include <resea.h>
#include <resea/channel.h>
#include <resea/pci.h>


handler_t pci_handler;

extern "C" result_t pci_startup(void) {
    channel_t ch;
    result_t r;

    ch = create_channel();
    resea::interfaces::channel::call_register(connect_to_local(1), ch,
        INTERFACE(pci), &r);

    serve_channel(ch, pci::server_handler);
}
