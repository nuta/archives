#include "pci.h"
#include <resea.h>
#include <resea/cpp/memory.h>
#include <resea/pci.h>
#include "pci.h"


/** handles pci.listen */
void pci_pci_listen(channel_t __ch, channel_t ch, uint32_t vendor, uint32_t device,
                    uint32_t subvendor, uint32_t subdevice) {

    // TODO
    send_pci_listen_reply(__ch, OK);

    void *config = allocate_memory(PCI_CONFIG_HEADER_SIZE, MEMORY_ALLOC_NORMAL);
    if (pci_lookup(config, vendor, device, subvendor, subdevice) == OK) {
        INFO("found a device");
        sendas_pci_new_device(ch, config, PAYLOAD_MOVE_OOL, PCI_CONFIG_HEADER_SIZE, PAYLOAD_INLINE);
    } else {
        INFO("device not found");
    }
}
