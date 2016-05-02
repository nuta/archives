#ifndef __TCPIP_ENDIAN_H__
#define __TCPIP_ENDIAN_H__

#include "types.h"

uint16_t tcpip_to_host_endian16(uint16_t x);
uint32_t tcpip_to_host_endian32(uint32_t x);
uint16_t tcpip_to_net_endian16(uint16_t x);
uint32_t tcpip_to_net_endian32(uint32_t x);

#endif
