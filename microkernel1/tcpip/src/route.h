#ifndef __TCPIP_ROUTE_H__
#define __TCPIP_ROUTE_H__

#include "device.h"

namespace tcpip {

struct net_device *route(struct addr *addr);

} // namespace tcpip

#endif
