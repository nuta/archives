#include "virtio.h"
#include <resea.h>
#include <resea/virtio.h>
#include <resea/net_device.h>
#include <resea/storage_device.h>
#include "virtio_blk.h"


/** handles storage_device.write */
void virtio_storage_device_write(channel_t __ch, offset_t offset, void * data, size_t size) {

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

    send_storage_device_write_reply(__ch, result);
}
