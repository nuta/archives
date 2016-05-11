#include <resea.h>
#include <resea/channel.h>
#include <resea/pci.h>
#include <resea/storage_device.h>
#include <resea/cpp/io.h>
#include <resea/cpp/memory.h>
#include <string.h>
#include <libpci/pci.h>
#include "virtio.h"
#include "virtio_blk.h"

static struct virtio_device device;


static void new_device_handler(channel_t ch, payload_t *m) {
    uint32_t feats, bar0;
    result_t r;
    struct libpci_config_header *config;

    switch (EXTRACT_MSGID(m)) {
    case MSGID(pci, new_device):
        config = (struct libpci_config_header *) EXTRACT(m, pci, new_device, header);
        bar0   = config->bar0;

        DEBUG("virtio-blk device found (%x:%x)",
            config->vendor, config->device);
  
        // setup the device
        virtio_setup_device(&device, bar0);
        feats = virtio_get_features(&device);
        virtio_set_features(&device, feats);
        virtio_init_queue(&device, VIRTIO_BLK_RQUEUE);
        virtio_activate_device(&device);

        DEBUG("virtio-blk: device ready");
        call_channel_register(connect_to_local(1), virtio_server,
            INTERFACE(storage_device), &r);
        break;
    }
}


void virtio_blk_init(void){
    channel_t pci_server, pci_client;
    result_t r;

    pci_server = create_channel();
    call_channel_connect(connect_to_local(1), pci_server, INTERFACE(pci), &r);

    pci_client = create_channel();
    set_channel_handler(pci_client, new_device_handler);

    call_pci_listen(pci_server, pci_client,
        VIRTIO_PCI_VENDOR, PCI_ID_ANY,
        PCI_ID_ANY, VIRTIO_PCI_SUBSYS_BLOCK,
        &r);
}


result_t virtio_blk_write(uintmax_t sector, size_t n, const void *data) {
    struct virtio_request rs[3];
    struct virtio_blk_request_header *header;
    size_t data_size;
    uint8_t *status;
    uintptr_t status_addr, buf_addr;
    paddr_t buf_paddr, header_paddr, status_paddr;
    result_t r;
  
    data_size = n * VIRTIO_BLK_SECTOR_SIZE;

    DEBUG("write: offset=%#0x, size=%d", sector * VIRTIO_BLK_SECTOR_SIZE, data_size);
  
    call_memory_allocate_physical(get_memory_ch(),
        0, sizeof(*header), MEMORY_ALLOC_CONTINUOUS,
        &r, (uintptr_t *) &header, &header_paddr);

    call_memory_allocate_physical(get_memory_ch(),
        0, data_size, MEMORY_ALLOC_CONTINUOUS,
        &r, &buf_addr, &buf_paddr);

    call_memory_allocate_physical(get_memory_ch(),
        0, sizeof(*status), MEMORY_ALLOC_CONTINUOUS,
        &r, &status_addr, &status_paddr);

    header->type   = VIRTIO_BLK_WRITE;
    header->sector = sector;
    rs[0].data  = (uint64_t) header_paddr;
    rs[0].size  = sizeof(*header);
    rs[0].flags = 0; // READONLY
  
    memcpy((void *) buf_addr, data, data_size);
    INFO("buf=%p, data=%p, size=%d (%s)", buf_addr, data, data_size, data);
    rs[1].data  = (uint64_t) buf_paddr;
    rs[1].size  = data_size;
    rs[1].flags = 0; // READONLY
  
    rs[2].data  = (uint64_t) status_paddr;
    rs[2].size  = sizeof(*status);
    rs[2].flags = VIRTIO_DESC_F_WRITE;
  
    virtio_send_request(&device, VIRTIO_BLK_RQUEUE, (struct virtio_request *) &rs, 3);
    return OK;
}


result_t virtio_blk_read(uintmax_t sector, size_t n, void **data) {
    struct virtio_request rs[3];
    struct virtio_blk_request_header *header;
    size_t data_size;
    uint8_t *status;
    paddr_t paddr, header_paddr, status_paddr;
    uintptr_t buf_addr, status_addr;
    result_t r;
  
    data_size = n * VIRTIO_BLK_SECTOR_SIZE;
 
    DEBUG("read: offset=%#0x, size=%d", sector * VIRTIO_BLK_SECTOR_SIZE, data_size);

    call_memory_allocate_physical(get_memory_ch(),
        0, sizeof(*header), MEMORY_ALLOC_CONTINUOUS,
        &r, (uintptr_t *) &header, &header_paddr);

    call_memory_allocate_physical(get_memory_ch(),
        0, data_size, MEMORY_ALLOC_CONTINUOUS,
        &r, &buf_addr, &paddr);

    call_memory_allocate_physical(get_memory_ch(),
        0, sizeof(*status), MEMORY_ALLOC_CONTINUOUS,
        &r, &status_addr, &status_paddr);

    header->type   = VIRTIO_BLK_READ;
    header->sector = sector;
    rs[0].data  = (uint64_t) header_paddr;
    rs[0].size  = sizeof(*header);
    rs[0].flags = 0; // READONLY
  
    rs[1].data  = (uint64_t) paddr;
    rs[1].size  = data_size;
    rs[1].flags = VIRTIO_DESC_F_WRITE;
  
    rs[2].data  = (uint64_t) status_paddr;
    rs[2].size  = sizeof(*status);
    rs[2].flags = VIRTIO_DESC_F_WRITE;
  
    virtio_send_request(&device, VIRTIO_BLK_RQUEUE, (struct virtio_request *) &rs, 3);
  
    *data = (void *) buf_addr;
    return OK;
}
