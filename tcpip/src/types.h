#ifndef __TCPIP_TYPES_H__
#define __TCPIP_TYPES_H__

#include <resea.h>

#define TCPIP_LITTLE_ENDIAN __ORDER_LITTLE_ENDIAN__
#define TCPIP_PACKED __attribute__((packed))

#define TCPIP_PORT_ANY       0
typedef uint16_t tcpip_port_t;

typedef enum {
    TCPIP_LINKTYPE_ARP  = 0,
    TCPIP_LINKTYPE_IPV4 = 1,
    TCPIP_LINKTYPE_ICMP = 2,
    TCPIP_LINKTYPE_MAX  = 3
} tcpip_link_type_t;

typedef enum {
    TCPIP_IPV4 = 1 << 0,
    TCPIP_IPV6 = 1 << 1,
    TCPIP_UDP  = 1 << 2,
    TCPIP_TCP  = 1 << 3,
} tcpip_protocol_t;

struct tcpip_addr {
    tcpip_protocol_t protocol;
    uint16_t port;

    union {
        struct {
            uint32_t ipv4_addr;
        };
    };
};

#endif
