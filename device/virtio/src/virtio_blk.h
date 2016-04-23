#ifndef __VIRTIO_BLK_H__
#define __VIRTIO_BLK_H__

struct virtio_blk_request_header{
  uint32_t type;     /* VIRTIO_READ or VIRTIO_BLK_WRITE */
  uint32_t priority; /* priority (0 is lowest)  */
  uint64_t sector;   /* where to read/write */
} PACKED;


#define VIRTIO_BLK_RQUEUE 0
#define VIRTIO_BLK_READ  0
#define VIRTIO_BLK_WRITE 1
#define VIRTIO_BLK_SECTOR_SIZE 512 // FIXME

void virtio_blk_init(void);
result_t virtio_blk_write(uintmax_t sector, size_t n, const void *data);
result_t virtio_blk_read(uintmax_t sector, size_t n, void **data);

#endif
