#ifndef __TCPIP_ROUTE_H__
#define __TCPIP_ROUTE_H__

#include "device.h"

struct net_device *tcpip_route(struct addr *addr);

#endif
