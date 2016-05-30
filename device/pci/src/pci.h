#ifndef __PCI_H__
#define __PCI_H__

#define PACKAGE_NAME "pci"

#include <resea.h>

/* PCI */
#define PCI_CONFIG_VENDOR     0x00
#define PCI_CONFIG_DEVICEID   0x02
#define PCI_CONFIG_SUBVENDOR  0x2c
#define PCI_CONFIG_SUBDEVICE  0x2e

#define PCI_CONFIG_HEADER_SIZE 0x40

result_t pci_lookup(void *config, uint32_t vendor, uint32_t device,
                    uint32_t subvendor, uint32_t subdevice);
#endif
