#ifndef __TCPIP_IPV4_H__
#define __TCPIP_IPV4_H__

#include "types.h"
#include "instance.h"

struct tcpip_ipv4_header {
    uint8_t   version;
    uint8_t   tos;
    uint16_t  length;
    uint16_t  id;
    uint16_t  frag_offset;
    uint8_t   ttl;
    uint8_t   proto;
    uint16_t  checksum;
    uint32_t  src_addr;
    uint32_t  dest_addr;
} TCPIP_PACKED;

#define TCPIP_IPTYPE_ICMP  0x01
#define TCPIP_IPTYPE_UDP   0x11

#define TCPIP_IPV4_ADDR_ANY  0

void tcpip_receive_ipv4(struct tcpip_instance *instance, const void *payload, size_t size);

#endif

