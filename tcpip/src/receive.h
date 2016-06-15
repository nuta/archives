#ifndef __TCPIP_RECEIVE_H__
#define __TCPIP_RECEIVE_H__

#include "types.h"

namespace tcpip {

void receive_packet(net_type_t type, const void *payload, size_t size);

} // namespace tcpip

#endif
