#include <resea.h>
#include <resea/cpp/io.h>
#include <resea/cpp/memory.h>
#include <string.h>
#include "virtio.h"
#include "virtio_blk.h"
#include "pci.h"

static struct virtio_device device;

void virtio_blk_test(void);

bool virtio_blk_init(void){
    struct pci_device pci;
    uint32_t feats;
  
    if (!pci_lookup(&pci, VIRTIO_PCI_VENDOR, 0x1001 /* XXX */,
                    VIRTIO_PCI_SUBSYS_BLOCK)){
        DEBUG("virtio-blk device not found");
        return false;
    }
  
    DEBUG("virtio-blk device found: PCI bus=%d, dev=%d", pci.bus, pci.dev);
  
    /* setup the device */
    virtio_setup_device(&device, &pci);
    feats = virtio_get_features(&device);
    virtio_set_features(&device, feats);
    virtio_init_queue(&device, VIRTIO_BLK_RQUEUE);
    virtio_activate_device(&device);

    DEBUG("virtio-blk: device ready");

//    virtio_blk_test();
    return true;
}


result_t virtio_blk_write(uintmax_t sector, size_t n, const void *data) {
    struct virtio_request rs[3];
    struct virtio_blk_request_header *header;
    size_t data_size;
    uint8_t *status;
    paddr_t paddr, header_paddr, status_paddr;
    void *buf;
  
    data_size = n * VIRTIO_BLK_SECTOR_SIZE;

    DEBUG("write: offset=%#0x, size=%d", sector * VIRTIO_BLK_SECTOR_SIZE, data_size);
  
    header = allocPhysicalMemory(0, sizeof(*header), MEMORY_ALLOCMEM_CONTINUOUS,
                                 &header_paddr);
    header->type   = VIRTIO_BLK_WRITE;
    header->sector = sector;
    rs[0].data  = (uint64_t) header_paddr;
    rs[0].size  = sizeof(*header);
    rs[0].flags = 0; // READONLY
  
    buf = allocPhysicalMemory(0, data_size, MEMORY_ALLOCMEM_CONTINUOUS, &paddr);
    memcpy(buf, data, data_size);
    INFO("buf=%p, data=%p, size=%d (%s)", buf, data, data_size, data);
    rs[1].data  = (uint64_t) paddr;
    rs[1].size  = data_size;
    rs[1].flags = 0; // READONLY
  
    allocPhysicalMemory(0, sizeof(*status), MEMORY_ALLOCMEM_CONTINUOUS, &status_paddr);
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
    void *buf;
  
    data_size = n * VIRTIO_BLK_SECTOR_SIZE;
 
    DEBUG("read: offset=%#0x, size=%d", sector * VIRTIO_BLK_SECTOR_SIZE, data_size);
 
    header = allocPhysicalMemory(0, sizeof(*header), MEMORY_ALLOCMEM_CONTINUOUS,
                                 &header_paddr);
    header->type   = VIRTIO_BLK_READ;
    header->sector = sector;
    rs[0].data  = (uint64_t) header_paddr;
    rs[0].size  = sizeof(*header);
    rs[0].flags = 0; // READONLY
  
    buf = allocPhysicalMemory(0, data_size, MEMORY_ALLOCMEM_CONTINUOUS, &paddr);
    rs[1].data  = (uint64_t) paddr;
    rs[1].size  = data_size;
    rs[1].flags = VIRTIO_DESC_F_WRITE;
  
    allocPhysicalMemory(0, sizeof(*status), MEMORY_ALLOCMEM_CONTINUOUS, &status_paddr);
    rs[2].data  = (uint64_t) status_paddr;
    rs[2].size  = sizeof(*status);
    rs[2].flags = VIRTIO_DESC_F_WRITE;
  
    virtio_send_request(&device, VIRTIO_BLK_RQUEUE, (struct virtio_request *) &rs, 3);
  
    *data = buf;
    return OK;
}


void virtio_blk_test(void){
    uint8_t buf[512];
    void *data;
    const char *test_str = "Hello virtio-blk!";
  
    strcpy_s((char *) &buf, sizeof(buf), test_str);
    DEBUG("storage_test: writing");
    virtio_blk_write(0, 1, test_str);
    DEBUG("storage_test: reading");
    virtio_blk_read(0, 1, &data);
    DEBUG("storage_test: %s (read data: \"%s\")",
            (!strcmp((const char *) data, test_str))? "OK" : "FAIL", data);
}
