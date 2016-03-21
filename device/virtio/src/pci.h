#ifndef __PCI_H__
#define __PCI_H__

#include <resea.h>

/* PCI */
#define PCI_CONFIG_VENDOR   0x00
#define PCI_CONFIG_DEVICEID 0x02
#define PCI_CONFIG_BAR0     0x10 /* base address #0 */
#define PCI_CONFIG_SUBSYS   0x2e

struct pci_device{
  uint16_t bus, dev;
};

int pci_lookup(struct pci_device *pci, uint16_t vendor, uint16_t deviceid, uint16_t subsys);
uint16_t pci_read_config16(struct pci_device *pci, offset_t offset);
uint32_t pci_read_config32(struct pci_device *pci, offset_t offset);

#endif
