#include "tcpip.h"
#include <resea.h>
#include <resea/channel.h>
#include <resea/tcpip.h>
#include "socket.h"

bool tcpip_initialized = false;

handler_t tcpip_handler;

void tcpip_startup() {
    result_t r;
    channel_t ch;

    tcpip_init_socket();

    ch = create_channel();
    call_channel_register(connect_to_local(1), ch,
        INTERFACE(tcpip), &r);

    tcpip_initialized = true;
    serve_channel(ch, tcpip_handler);
}
