#include "pci.h"
#include <resea.h>
#include <resea/cpp/memory.h>
#include <resea/cpp/io.h>

#define PCI_IO_ADDR 0x0cf8
#define PCI_IO_DATA 0x0cfc


static uint16_t _pci_read_config16(uint16_t bus, uint16_t dev, offset_t offset){
  uint32_t addr;
  uint32_t data;

  /* NOTE: offset must be multiple of 4 */
  addr = (uint32_t) ((1 << 31) | (bus << 16) | (dev << 11) | (offset & 0xfc));

  io_write32(IO_SPACE_PORT, 0, PCI_IO_ADDR, addr);
  data = io_read32(IO_SPACE_PORT,  0, PCI_IO_DATA);

  return (uint16_t) (data >> (offset & 3) * 8);
}


uint16_t pci_read_config16(struct pci_device *pci, offset_t offset){

  return _pci_read_config16(pci->bus, pci->dev, offset);
}


static uint32_t _pci_read_config32(uint16_t bus, uint16_t dev, offset_t offset){
  uint32_t addr;
  uint32_t data;

  /* NOTE: offset must be multiple of 4 */
  addr = (uint32_t) ((1 << 31) | (bus << 16) | (dev << 11) | (offset & 0xfc));

  io_write32(IO_SPACE_PORT, 0, PCI_IO_ADDR, addr);
  data = io_read32(IO_SPACE_PORT,  0, PCI_IO_DATA);

  return (uint32_t) data;
}


uint32_t pci_read_config32(struct pci_device *pci, offset_t offset){

  return _pci_read_config32(pci->bus, pci->dev, offset);
}


int pci_lookup(struct pci_device *pci, uint16_t vendor, uint16_t deviceid,
               uint16_t subsys){
  int bus, dev;

  /* look for the device (brute force) */
  for(bus=0; bus < 256; bus++){
    for(dev=0; dev < 32; dev++){
      // check vendor ID
      if(_pci_read_config16(bus, dev, PCI_CONFIG_VENDOR) != vendor)
        continue;
      // check device ID
      if(_pci_read_config16(bus, dev, PCI_CONFIG_DEVICEID) != deviceid)
        continue;
      // check subsystem ID
      if(_pci_read_config16(bus, dev, PCI_CONFIG_SUBSYS) != subsys)
        continue;

      // found!
      pci->bus = bus;
      pci->dev = dev;
      return 1;
    }
  }

  // not found
  return 0;
}
