#include "pci.h"
#include <resea.h>
#include <resea/pci.h>

namespace pci {
namespace pci_server {

void handle_listen(
    channel_t __ch
    , channel_t ch
    , uint32_t vendor
    , uint32_t device
    , uint32_t subvendor
    , uint32_t subdevice
);

} // namespace pci
} // namespace pci
