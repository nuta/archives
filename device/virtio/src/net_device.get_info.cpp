#include "virtio.h"
#include "virtio_net.h"
#include <resea.h>
#include <resea/virtio.h>
#include <resea/storage_device.h>
#include <resea/net_device.h>
#include <resea/cpp/memory.h>


namespace virtio {
namespace net_device_server {

/** handles net_device.get_info */
void handle_get_info(
    channel_t __ch
) {
    uint8_t* hwaddr;

    hwaddr = (uint8_t *) allocate_memory(6, resea::interfaces::memory::ALLOC_NORMAL);
    virtio_net_get_hwaddr(hwaddr);

    resea::interfaces::net_device::sendas_get_info_reply(__ch,
        OK, PAYLOAD_INLINE,
        resea::interfaces::net_device::HW_ETHERNET, PAYLOAD_INLINE,
        hwaddr, PAYLOAD_MOVE_OOL,             // hwaddr
        6, PAYLOAD_INLINE,
        MAX_DATA_SIZE, PAYLOAD_INLINE         // max_data_size
        );
}

} // namespace net_device_server
} // namespace virtio
