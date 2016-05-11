#ifndef __TCPIP_TYPES_H__
#define __TCPIP_TYPES_H__

#include <resea.h>
#include <resea/tcpip.h>

#define TCPIP_LITTLE_ENDIAN __ORDER_LITTLE_ENDIAN__
#define PACKED __attribute__((packed))

#define PORT_ANY       0
typedef uint16_t tcpip_port_t;

typedef enum {
    LINKTYPE_ARP  = 0,
    LINKTYPE_IPV4 = 1,
    LINKTYPE_ICMP = 2,
    LINKTYPE_MAX  = 3
} tcpip_link_type_t;

struct addr {
    tcpip_protocol_t protocol;
    uint16_t port;

    union {
        struct {
            uint32_t ipv4_addr;
        };
    };
};

#endif
