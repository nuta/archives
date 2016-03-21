#ifndef __VIRTIO_H__
#define __VIRTIO_H__

#include <resea.h>

#define VIRTIO_PCI_VENDOR       0x1af4
#define VIRTIO_PCI_DEVICEID     0x1000
#define VIRTIO_PCI_SUBSYS_NET   1
#define VIRTIO_PCI_SUBSYS_BLOCK 2

#define VIRTIO_IO_DEVICE_FEATS   0x00
#define VIRTIO_IO_GUEST_FEATS    0x04
#define VIRTIO_IO_DEVICE_STATUS  0x12
#define VIRTIO_IO_DEVICE_SPECIFIC  0x14
#define VIRTIO_IO_QUEUE_SELECT   0x0e
#define VIRTIO_IO_QUEUE_ADDR     0x08
#define VIRTIO_IO_QUEUE_NOTIFY   0x10
#define VIRTIO_IO_QUEUE_SIZE     0x0c

#define VIRTIO_STATUS_ACK          1
#define VIRTIO_STATUS_DRIVER       2
#define VIRTIO_STATUS_DRIVER_OK    4
#define VIRTIO_STATUS_FEATURES_OK  8

#define VIRTIO_DESC_F_NEXT  1
#define VIRTIO_DESC_F_WRITE 2 /* write-only (looking from the deivce) */

#define VIRTIO_QUEUE_NUM_MAX 4

struct virtio_request{
  uint64_t  data;
  uint32_t  size;
  uint16_t  flags;
} PACKED;

struct virtio_desc{
  uint64_t  addr;
  uint32_t  len;
  uint16_t  flags;
  uint16_t  next;
} PACKED;

struct virtio_used_ring{
  uint32_t index;
  uint32_t len;
} PACKED;

struct virtio_avail{
  uint16_t flags; /* 0/1: enable/disable interrupts */

  /* The index to be proceeded.
     It will be wraped at 65536 so we just increment this. */
  uint16_t index;

  /* the first element of an array of virtio_desc index */
  uint16_t ring;
} PACKED;

struct virtio_used{
  uint16_t flags;
  uint16_t index;
  struct virtio_used_ring ring; // the first element
} PACKED;

struct virtio_queue{
  struct virtio_desc  *desc;
  struct virtio_avail *avail;
  struct virtio_used  *used;
  uint16_t number;
  uint16_t queue_num;
  uint16_t last_used_index;
};

struct virtio_device{
  uint16_t iobase;
  uint8_t  iospace;
  struct virtio_queue queues[VIRTIO_QUEUE_NUM_MAX];
};

struct pci_device;
struct virtio_desc *virtio_get_used(struct virtio_device *device,
                                    int queue_index);
int virtio_send_request(struct virtio_device *device, int queue_index,
                        struct virtio_request *rs, int request_num);
int virtio_setup_device(struct virtio_device *device, struct pci_device *pci);
uint32_t virtio_get_features(struct virtio_device *device);
void virtio_set_features(struct virtio_device *device, uint32_t features);
int virtio_init_queue(struct virtio_device *device, int queue_index);
void virtio_activate_device(struct virtio_device *device);
void virtio_free_desc(struct virtio_device *device,
                      int queue_index,
                      struct virtio_desc *desc);

#endif
