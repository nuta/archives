#ifndef __TCPIP_ICMP_H__
#define __TCPIP_ICMP_H__

#include "types.h"
#include "instance.h"

struct tcpip_icmp_header {
    uint8_t   type;
    uint8_t   code;
    uint16_t  checksum;
    uint32_t  data;
} TCPIP_PACKED;

#define TCPIP_ICMPTYPE_ECHO_REPLY    0
#define TCPIP_ICMPTYPE_ECHO_REQUEST  8

void tcpip_receive_icmp(struct tcpip_instance *instance,
  struct tcpip_addr *src_addr, struct tcpip_addr *dest_addr,
  const void *payload, size_t size);

#endif

