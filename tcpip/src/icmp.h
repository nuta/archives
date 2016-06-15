#ifndef __TCPIP_ICMP_H__
#define __TCPIP_ICMP_H__

#include "types.h"
#include "mbuf.h"

namespace tcpip {

struct icmp_header {
    uint8_t   type;
    uint8_t   code;
    uint16_t  checksum;
    uint32_t  data;
} PACKED;

const uint8_t ICMPTYPE_ECHO_REPLY   = 0;
const uint8_t ICMPTYPE_ECHO_REQUEST = 8;

void receive_icmp(struct addr *src_addr, struct addr *dest_addr,
                  struct mbuf *mbuf);

} // namespace tcpip

#endif

