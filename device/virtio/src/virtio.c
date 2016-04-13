#include "virtio.h"
#include "pci.h"
#include <resea.h>
#include <resea/io.h>


/**
 *  Allocates a virtio_desc
 *
 *  @param[in] queue  The virtio_queue where a virtio_desc allocated from.
 *  @return  The index of descriptor on success or -1 on failure.
 */
static int alloc_desc(struct virtio_queue *queue) {

    /* allocate a virtio_desc */
    for (int i=0; i < queue->queue_num; i++) {
        if (queue->desc[i].len == 0) {
            queue->desc[i].len = 1; // mark as reserved
            return i; // TODO: add lock into the virtio_desc
        }
    }
  
    WARN("cannot allocate a virtio_desc");
    return -1;
}


/**
 *  Frees a virtio_desc
 *
 *  @param[in] device       The virito_device.
 *  @param[in] queue_index  The index of queue.
 *  @param[in] desc         The descriptor to be freed.
 */
void virtio_free_desc(struct virtio_device *device,
                      int queue_index,
                      struct virtio_desc *desc) {
  struct virtio_queue *queue = &device->queues[queue_index];

    while (true) {
        desc->len = 0;
        if (!(desc->flags & VIRTIO_DESC_F_NEXT))
            break;
        desc = &queue->desc[desc->next];
    }
}


/**
 *  Returns a used descriptor
 *
 *  @param[in] device       The virtio_device.
 *  @param[in] queue_index  The index of queue.
 *  @return  The descriptor used by device or NULL if there are no one.
 */
struct virtio_desc *virtio_get_used(struct virtio_device *device,
                                    int queue_index) {
    struct virtio_queue *queue = &device->queues[queue_index];
    struct virtio_used_ring *used_ring = &queue->used->ring;
    uint16_t used_index;

    used_index = queue->used->index % queue->queue_num;

    if (queue->last_used_index != used_index) {
        used_index--; /* XXX */
        queue->last_used_index++;
        return &queue->desc[used_ring[used_index].index];
    } else {
        return NULL;
    }
}


/**
 *  Add requests into the avail ring and notify the device
 *
 *  @param[in] device       The virito_device.
 *  @param[in] queue_index  The index of queue.
 *  @param[in] rs           The array of requests.
 *  @param[in] rs_num       The number of requests.
 *  @return  Returns 0 on success.
 */
int virtio_send_request(struct virtio_device *device, int queue_index,
                        struct virtio_request *rs, int rs_num) {
    int desc_index, next_desc_index, first_desc_index, used_index;
    struct virtio_queue *queue = &device->queues[queue_index];
    uint16_t *avail_ring = &queue->avail->ring;
  
    if (rs_num < 0)
        return 1;
  
    desc_index = alloc_desc(queue);
    first_desc_index = desc_index;
  
    for (int i=0; i < rs_num; i++) {
  
        /* does the next desc exist? */
        if (i < rs_num - 1) {
          next_desc_index = alloc_desc(queue);
          queue->desc[desc_index].flags = (rs[i].flags | VIRTIO_DESC_F_NEXT);
        } else {
          // the final desc
          next_desc_index = 0;
          queue->desc[desc_index].flags = rs[i].flags;
        }
    
        queue->desc[desc_index].addr = rs[i].data;
        queue->desc[desc_index].len  = rs[i].size;
        queue->desc[desc_index].next = next_desc_index;
        desc_index = next_desc_index;
    }
  
    /* append the index of the first queue_desc to avail ring */
    avail_ring[queue->avail->index % queue->queue_num] = first_desc_index;
  
    /* increment the index in avail_ring */
    MEMORY_BARRIER();
    queue->avail->index++;
  
    used_index = queue->used->index;
    /* notify the device */
    io_write16(device->iospace, device->iobase,
              VIRTIO_IO_QUEUE_NOTIFY,
              queue->number);

    /* wait for completion
       TODO: use interrupt */
    while (queue->used->index == used_index);

    virtio_free_desc(device, queue->number, &queue->desc[desc_index]);
    return 0;
}


/**
 *  Initializes a virtio device
 *
 *  To setup a virtio device, you should follow the following steps:
 *
 *  1. virtio_setup_device()
 *  2. do feature negotiation using virtio_get_features() and virtio_set_features()
 *  3. virtio_init_queue() for each queue
 *  4. virtio_activate_device()
 *
 *  @param[in] device  The virtio_device.
 *  @param[in] pci     The pci_device of the virtio device.
 *  @returns  0 on success or 1 on fail
 */
int virtio_setup_device(struct virtio_device *device, struct pci_device *pci){
    uint32_t bar0;
  
    /* get iospace and iobase from BAR0 in the PCI config space */
    bar0 = pci_read_config32(pci, PCI_CONFIG_BAR0);
    device->iobase  = bar0 & 0xfffffffc;
    device->iospace = (bar0 & 1)? IO_IOSPACE_PORT : IO_IOSPACE_MEM;
  
    /* reset */
    io_write8(device->iospace, device->iobase, VIRTIO_IO_DEVICE_STATUS, 0x00);
    io_read8(device->iospace, device->iobase, VIRTIO_IO_DEVICE_STATUS);
  
    /* tell to the device that we know how to use */
    io_write8(device->iospace, device->iobase, VIRTIO_IO_DEVICE_STATUS,
             VIRTIO_STATUS_ACK | VIRTIO_STATUS_DRIVER);
  
    return 0;
}


/**
 *  Returns features of a device
 *
 *  @param[in] virtio_device  The device.
 *  @return  The features supported by the device.
 */
uint32_t virtio_get_features(struct virtio_device *device) {
  
    return io_read32(device->iospace, device->iobase, VIRTIO_IO_DEVICE_FEATS);
}


/**
 *  Set features to a device
 *
 *  @param[in] device    The device.
 *  @param[in] features  The features we want to use.
 */
void virtio_set_features(struct virtio_device *device, uint32_t features) {
    uint8_t old;
  
    io_write32(device->iospace, device->iobase, VIRTIO_IO_GUEST_FEATS, features);
  
    /* feature negotiation finished */
    old = io_read8(device->iospace, device->iobase, VIRTIO_IO_DEVICE_STATUS);
    io_write8(device->iospace, device->iobase, VIRTIO_IO_DEVICE_STATUS,
             VIRTIO_STATUS_FEATURES_OK | old);
}


/**
 *  Activates a device
 *
 *  @param[in] device  The device to be activated.
 */
void virtio_activate_device(struct virtio_device *device) {
    uint8_t old;
  
    old = io_read8(device->iospace, device->iobase, VIRTIO_IO_DEVICE_STATUS);
    io_write8(device->iospace, device->iobase, VIRTIO_IO_DEVICE_STATUS,
             VIRTIO_STATUS_DRIVER_OK | old);
}


/**
 *  Initializes a queue of a device
 *
 *  @param[in] device       The device.
 *  @param[in] queue_index  The index of the queue to be initialized.
 *  @return  0 on success or 1 on fail.
 */
int virtio_init_queue(struct virtio_device *device, int queue_index) {
    struct virtio_queue *queue = &device->queues[queue_index];
    size_t queue_size;
    void *buf;
    paddr_t phy_buf;
    uintptr_t addr;
  
    /* get the number of queue */
    io_write16(device->iospace, device->iobase, VIRTIO_IO_QUEUE_SELECT, queue_index);
    queue->queue_num = io_read16(device->iospace, device->iobase, VIRTIO_IO_QUEUE_SIZE);
  
    if (queue->queue_num == 0) {
        WARN("the number of queue #%d is 0", queue_index);
        return 1;
    }
  
    queue_size = sizeof(struct virtio_queue) + sizeof(uint16_t)*2 +
                 sizeof(uint16_t)*queue->queue_num*2 +
                 sizeof(struct virtio_desc)*queue->queue_num;
  
    buf = allocPhysicalMemory(0, queue_size,
                              ALLOCMEM_PAGE_ALIGNED | ALLOCMEM_CONTINUOUS, &phy_buf);
  
    queue->number = queue_index;
  
    /* virtio_desc */
    addr = PTR2ADDR(buf);
    queue->desc = (void *) addr;
  
    /* avail_ring */
    addr += sizeof(struct virtio_desc) * queue->queue_num;
    queue->avail = (void *) addr;
    queue->avail->index = 0;
    queue->avail->flags = 0;
  
    /* used_ring */
    addr = ALIGN(addr + sizeof(uint16_t) * (3 + queue->queue_num), 0x1000);
    queue->used = (void *) addr;
    queue->used->index = 0;
    queue->used->flags = 0;
    queue->last_used_index = 0;
  
    /* tell the device the physical address of the queue */
    io_write32(device->iospace, device->iobase, VIRTIO_IO_QUEUE_ADDR, phy_buf >> 12);
    return 0;
}
