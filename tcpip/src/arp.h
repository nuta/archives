#ifndef __TCPIP_ARP_H__
#define __TCPIP_ARP_H__

#include "types.h"

#define TCPIP_ARP_REQUEST  1 
#define TCPIP_ARP_REPLY    2

struct tcpip_arp_header {
    uint16_t  hw_type;
    uint16_t  proto_type;
    uint8_t   hw_len;
    uint8_t   proto_len;
    uint16_t  op;
    uint8_t   sender_hw_addr[6];
    uint32_t  sender_proto_addr;
    uint8_t   target_hw_addr[6];
    uint32_t  target_proto_addr;
} TCPIP_PACKED;

void tcpip_receive_arp(const void *payload, size_t size);

#endif
