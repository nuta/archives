#include <string.h>
#include "virtio.h"
#include <resea.h>
#include <resea/channel.h>
#include <resea/net_device.h>
#include <resea/pci.h>
#include <resea/cpp/io.h>
#include <resea/cpp/memory.h>
#include <libpci/pci.h>
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
    result_t r;

    header.gso_type    = 0;
    header.hdr_len     = 0;
    header.gso_size    = 0;
    header.csum_start  = 0;
    header.csum_offset = 0;

    rs[0].data  = (uint64_t) PTR2ADDR(&header);
    rs[0].size  = sizeof(header);
    rs[0].flags = 0; // READONLY

    call_memory_allocate_physical(get_memory_ch(),
        0, size, MEMORY_ALLOC_CONTINUOUS,
        &r, &addr, &paddr);

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
    while((desc = virtio_get_used(&device, VIRTIO_NET_RECV_QUEUE)) == nullptr);

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
    *data = allocate_memory(*size, MEMORY_ALLOC_NORMAL);

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


static void new_device_handler(channel_t ch, payload_t *m) {
    uint32_t feats, bar0;
    result_t r;
    struct libpci_config_header *config;

    switch (EXTRACT_MSGID(m)) {
    case MSGID(pci, new_device):
        config = (struct libpci_config_header *) EXTRACT(m, pci, new_device, header);
        bar0   = config->bar0;

        DEBUG("virtio-net device found (%x:%x)",
            config->vendor, config->device);

        // setup the device
        virtio_setup_device(&device, bar0);
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

        call_memory_allocate_physical(get_memory_ch(),
            0, 0x800 * fill_num, MEMORY_ALLOC_CONTINUOUS,
            &r, &dma_addr, &dma_paddr);

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
        call_channel_register(connect_to_local(1), virtio_server,
            INTERFACE(net_device), &r);
        break;
    }
}


void virtio_net_init(void){
    channel_t pci_server, pci_client;
    result_t r;

    pci_server = create_channel();
    call_channel_connect(connect_to_local(1), pci_server, INTERFACE(pci), &r);

    pci_client = create_channel();
    set_channel_handler(pci_client, new_device_handler);

    call_pci_listen(pci_server, pci_client,
        VIRTIO_PCI_VENDOR, PCI_ID_ANY,
        PCI_ID_ANY, VIRTIO_PCI_SUBSYS_NET,
        &r);
}
