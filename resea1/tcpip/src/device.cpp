#include "tcpip.h"
#include "device.h"
#include "ethernet.h"
#include <resea.h>
#include <resea/cpp/memory.h>
#include <resea/tcpip.h>
#include <resea/net_device.h>

using namespace tcpip;

namespace tcpip {

// TODO: isolate ethernet-dependent stuff
static struct net_device *devices = nullptr;
static size_t devices_max = 0;


// XXX
static struct net_device *get_device_by_channel(channel_t ch) {

    return &devices[0];
}

// XXX
struct net_device *route(struct addr *addr) {

    return devices;
}


static void net_device_client_handler(channel_t ch, payload_t *m) {
    void *data;
    size_t data_size;
    struct net_device *device;

    switch (EXTRACT_MSGID(m)) {
    case MSGID(net_device, received):
        data        = EXTRACT(m, net_device, received, data);
        data_size   = EXTRACT(m, net_device, received, data_size);
        device = get_device_by_channel(ch);
        device->receive(device, data, data_size);
        break;
    }
}


void add_device(channel_t ch) {
    result_t r;
    channel_t client_ch;

    client_ch = create_channel();
    set_channel_handler(client_ch, net_device_client_handler);
    resea::interfaces::net_device::call_listen(ch, client_ch, &r);

    if (r != OK) {
        WARN("failed to net_device.listen");
    }

    // XXX
    devices[0].ch            = ch;
    devices[0].hwaddr        = (void *) "\x00\x11\x22\x33\x44\x55";
    devices[0].hwaddr_len    = 6;
    devices[0].max_data_size = 1500;
    devices[0].addr.ipv4_addr = (10 << 24) | 15;
    devices[0].transmit      = ethernet_transmit;
    devices[0].receive       = ethernet_receive;
}


void init_devices() {

    devices_max = 8;
    devices = (struct net_device *) allocate_memory(
                  sizeof(struct net_device) * devices_max,
                  resea::interfaces::memory::ALLOC_ZEROED);
}

} // namespace tcpip
