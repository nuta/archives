#include "virtio.h"
#include "virtio_net.h"
#include <resea.h>
#include <resea/virtio.h>
#include <resea/storage_device.h>
#include <resea/net_device.h>

/** handles net_device.transmit */
void virtio_net_device_transmit(
    channel_t __ch
    , void * dest_hwaddr
    , size_t dest_hwaddr_size
    , void * src_hwaddr
    , size_t src_hwaddr_size
    , net_device_packet_type_t type
    , void * packet
    , size_t packet_size
) {
    result_t result;

    result = virtio_net_transmit(packet, packet_size); // TODO: add header
    send_net_device_transmit_reply(__ch, result);
}
