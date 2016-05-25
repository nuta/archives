#include "virtio.h"
#include <resea.h>
#include <resea/cpp/memory.h>
#include <resea/virtio.h>
#include <resea/net_device.h>
#include "virtio_net.h"


/** handles net_device.get_hardware_address */
void virtio_net_device_get_hardware_address(channel_t __ch) {
    uint8_t* hwaddr;

    hwaddr = (uint8_t *) allocate_memory(6, MEMORY_ALLOC_NORMAL);
    virtio_net_get_hwaddr(hwaddr);

    sendas_net_device_get_hardware_address_reply(__ch,
        OK, PAYLOAD_INLINE,
        hwaddr, PAYLOAD_MOVE_OOL,
	6, PAYLOAD_INLINE);
}
