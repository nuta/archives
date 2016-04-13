#include <resea.h>
#include <string.h>
#include "pci.h"
#include "virtio.h"
#include "virtio_net.h"

static uintptr_t dma_addr;
static paddr_t dma_paddr;
static uint8_t mac_addr[6];
static struct virtio_device device;


result_t virtio_net_transmit(void *data, size_t size) {
    struct virtio_request rs[2];
    struct virtio_net_request_header header;
    uintptr_t addr;
    paddr_t paddr;
  
    header.gso_type    = 0;
    header.hdr_len     = 0;
    header.gso_size    = 0;
    header.csum_start  = 0;
    header.csum_offset = 0;

    rs[0].data  = (uint64_t) PTR2ADDR(&header);
    rs[0].size  = sizeof(header);
    rs[0].flags = 0; // READONLY

    addr = PTR2ADDR(allocPhysicalMemory(0, size, ALLOCMEM_CONTINUOUS, &paddr));
    memcpy((void *) addr, data, size);

    rs[1].data  = paddr;
    rs[1].size  = size;
    rs[1].flags = 0; // READONLY

    virtio_send_request(&device, VIRTIO_NET_TRANS_QUEUE,
                        (struct virtio_request *) &rs, 2);

    DEBUG("TX: size=%d", size);
    return OK;
}


result_t virtio_net_receive(void **data, size_t *size) {
    struct virtio_desc *desc;

retry:
    DEBUG("wating for a packet");
    while((desc = virtio_get_used(&device, VIRTIO_NET_RECV_QUEUE)) == NULL);

    if (desc->addr == 0) {
        WARN("desc->addr is 0, retrying");
        goto retry;
    }

    if (desc->len == 0) {
        WARN("desc->size is 0, retrying");
        goto retry;
    }

    // addr is a physical address so we must convert it to a virtual address
    *size = desc->len;
    *data = allocMemory(*size, ALLOCMEM_NORMAL);

    /* the magic number '10' in 2nd arg. is the size of virtio-net's packet header */
    memcpy(*data,
           (void *) (dma_addr + (desc->addr - dma_paddr) + 10),
           *size);

    virtio_free_desc(&device, VIRTIO_NET_RECV_QUEUE, desc);

    DEBUG("RX: size=%d", *size);
    return OK;
}


void virtio_net_get_hwaddr(uint8_t *hwaddr) {

    for (int i=0; i < 6; i++)
        hwaddr[i] = mac_addr[i];
}


bool virtio_net_init(void){
  struct pci_device pci;
  uint32_t feats;

  if(!pci_lookup(&pci, VIRTIO_PCI_VENDOR, VIRTIO_PCI_DEVICEID,
                 VIRTIO_PCI_SUBSYS_NET)){
    INFO("virtio-net device not found");
    return false;
  }

  INFO("virtio-net device found: PCI bus=%d, dev=%d", pci.bus, pci.dev);

  /* setup the device */
  virtio_setup_device(&device, &pci);
  feats = virtio_get_features(&device);
  virtio_set_features(&device, feats);
  virtio_init_queue(&device, VIRTIO_NET_RECV_QUEUE);
  virtio_init_queue(&device, VIRTIO_NET_TRANS_QUEUE);
  virtio_init_queue(&device, VIRTIO_NET_CTRL_QUEUE);
  virtio_activate_device(&device);

  for(int i=0; i < 6; i++) 
    mac_addr[i] = io_read8(device.iospace, device.iobase, VIRTIO_IO_NET_MAC_ADDR + i);

  INFO("virtio-net: MAC address is %x:%x:%x:%x:%x:%x",
       mac_addr[0], mac_addr[1], mac_addr[2],
       mac_addr[3], mac_addr[4], mac_addr[5]);

  int fill_num = device.queues[VIRTIO_NET_RECV_QUEUE].queue_num;
  if (fill_num > 128)
      fill_num = 128;

  dma_addr = PTR2ADDR(allocPhysicalMemory(0, 0x800 * fill_num,
                                          ALLOCMEM_CONTINUOUS, &dma_paddr));

  INFO("virtio-net: filling avail_ring to receive packets (num=%d)", fill_num);
  INFO("virtio-net: avail_ring dma_addr=%p, dma_paddr=%p", dma_addr, dma_paddr);
  paddr_t paddr = dma_paddr;
  for (int i=0; i < fill_num; i++) {
    struct virtio_request rs;

    rs.data  = (uint64_t) paddr;
    rs.size  = 0x800; // FIXME
    rs.flags = VIRTIO_DESC_F_WRITE;

    virtio_send_request(&device, VIRTIO_NET_RECV_QUEUE,
                        (struct virtio_request *) &rs, 1);
    paddr += rs.size;
  }

  INFO("virtio-net: device ready");
  return true;
}
