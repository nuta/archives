#include "virtio.h"
#include <resea.h>

void virtio_storage_device_read(
    channel_t __ch
    , offset_t offset
    , size_t size
);
void virtio_storage_device_write(
    channel_t __ch
    , offset_t offset
    , void * data
    , size_t data_size
);
void virtio_net_device_listen(
    channel_t __ch
    , channel_t channel
);
void virtio_net_device_transmit(
    channel_t __ch
    , void * dest_hwaddr
    , size_t dest_hwaddr_size
    , void * src_hwaddr
    , size_t src_hwaddr_size
    , net_device_packet_type_t type
    , void * packet
    , size_t packet_size
);
void virtio_net_device_get_info(
    channel_t __ch
);
