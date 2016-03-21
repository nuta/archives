#include <resea.h>
#include <resea/virtio.h>
#include <resea/net_device.h>
#include <resea/storage_device.h>
#include "virtio_blk.h"


/** handles storage_device.read */
void virtio_storage_device_read(channel_t __ch, offset_t offset, size_t size){

    result_t result;
    void *data;
    if (offset % VIRTIO_BLK_SECTOR_SIZE != 0) {
        WARN("offset is not aligned to 0x%x", VIRTIO_BLK_SECTOR_SIZE);
        send_storage_device_read_reply(__ch, E_INVALID, NULL, 0);

    }else if (size % VIRTIO_BLK_SECTOR_SIZE != 0) {
        WARN("size is not aligned to 0x%x", VIRTIO_BLK_SECTOR_SIZE);
        send_storage_device_read_reply(__ch, E_INVALID, NULL, 0);

    } else {
        result = virtio_blk_read(offset / VIRTIO_BLK_SECTOR_SIZE,
                                 size / VIRTIO_BLK_SECTOR_SIZE,
                                 &data);
        send_storage_device_read_reply(__ch, result, MOVE(data), size);
    }
}
