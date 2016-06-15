#ifndef __TCPIP_ENDIAN_H__
#define __TCPIP_ENDIAN_H__

#include "types.h"

namespace tcpip {

uint16_t to_host_endian16(uint16_t x);
uint32_t to_host_endian32(uint32_t x);
uint16_t to_net_endian16(uint16_t x);
uint32_t to_net_endian32(uint32_t x);

} // namespce tcpip

#endif
