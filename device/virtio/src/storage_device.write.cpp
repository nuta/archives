#include "virtio.h"
#include <resea.h>
#include <resea/virtio.h>
#include <resea/net_device.h>
#include <resea/storage_device.h>
#include "virtio_blk.h"


namespace virtio {
namespace storage_device_server {

/** handles storage_device.write */
void handle_write(channel_t __ch, offset_t offset, void * data, size_t size) {

    result_t result;

    if (offset % VIRTIO_BLK_SECTOR_SIZE != 0) {
        WARN("offset is not aligned to 0x%x", VIRTIO_BLK_SECTOR_SIZE);
        result = E_INVALID;
    }

    if (size % VIRTIO_BLK_SECTOR_SIZE != 0) {
        WARN("size is not aligned to 0x%x", VIRTIO_BLK_SECTOR_SIZE);
        result = E_INVALID;
    }

    result = virtio_blk_write(offset / VIRTIO_BLK_SECTOR_SIZE,
                              size / VIRTIO_BLK_SECTOR_SIZE,
                              data);

    resea::interfaces::storage_device::send_write_reply(__ch, result);
}

} // namespace storage_device_server
} // namespace virtio
