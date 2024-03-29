#include "tcpip.h"
#include <resea.h>
#include <resea/channel.h>
#include <resea/tcpip.h>
#include <resea/net_device.h>
#include "handler.h"
#include "socket.h"
#include "device.h"
#include "arp.h"

using namespace tcpip;

extern "C" void tcpip_startup() {
    result_t r;
    channel_t ch, net_device_ch;

    INFO("starting");
    init_devices();
    init_arp();
    init_socket();

    net_device_ch = create_channel();
    resea::interfaces::channel::call_connect(connect_to_local(1), net_device_ch,
        INTERFACE(net_device), &r);

    INFO("connected to a net_device");
    add_device(net_device_ch);

    ch = create_channel();
    resea::interfaces::channel::call_register(connect_to_local(1), ch,
        INTERFACE(tcpip), &r);

    INFO("ready");
    serve_channel(ch, server_handler);
}
