#include <resea.h>
#include <resea/pci.h>
#include <resea/cpp/memory.h>
#include <resea/cpp/io.h>
#include "pci.h"

#define PCI_IO_ADDR 0x0cf8
#define PCI_IO_DATA 0x0cfc


static uint16_t read_config16(uint16_t bus, uint16_t dev, offset_t offset){
    uint32_t addr;
    uint32_t data;

    /* NOTE: offset must be multiple of 4 */
    addr = (uint32_t) ((1 << 31) | (bus << 16) | (dev << 11) | (offset & 0xfc));

    io_write32(IO_SPACE_PORT, 0, PCI_IO_ADDR, addr);
    data = io_read32(IO_SPACE_PORT,  0, PCI_IO_DATA);

    return (uint16_t) (data >> (offset & 3) * 8);
}


static uint32_t read_config32(uint16_t bus, uint16_t dev, offset_t offset){
    uint32_t addr;
    uint32_t data;
  
    /* NOTE: offset must be multiple of 4 */
    addr = (uint32_t) ((1 << 31) | (bus << 16) | (dev << 11) | (offset & 0xfc));
  
    io_write32(IO_SPACE_PORT, 0, PCI_IO_ADDR, addr);
    data = io_read32(IO_SPACE_PORT,  0, PCI_IO_DATA);
  
    return (uint32_t) data;
}


result_t pci_lookup(void *config, uint32_t vendor, uint32_t device,
                    uint32_t subvendor, uint32_t subdevice) {
    int bus, dev;

    /* look for the device (brute force) */
    for(bus=0; bus < 256; bus++){
      for(dev=0; dev < 32; dev++){
        // check vendor ID
        if(vendor != PCI_ID_ANY && read_config16(bus, dev, PCI_CONFIG_VENDOR) != vendor)
          continue;
        // check device ID
        if(device != PCI_ID_ANY && read_config16(bus, dev, PCI_CONFIG_DEVICEID) != device)
          continue;
        // check subsystem vendor ID
        if(subvendor != PCI_ID_ANY && read_config16(bus, dev, PCI_CONFIG_SUBVENDOR) != subvendor)
          continue;
        // check subsystem ID
        if(subdevice != PCI_ID_ANY && read_config16(bus, dev, PCI_CONFIG_SUBDEVICE) != subdevice)
          continue;

        // found!
        uint32_t *p = (uint32_t *) config;
        int offset = 0;

        while (offset < PCI_CONFIG_HEADER_SIZE) {
            *p = read_config32(bus, dev, offset);
            offset += 4;
            p++;
        }

        return OK;
      }
    }

    // not found
    return E_NOTFOUND;
}
