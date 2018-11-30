#ifndef __TCPIP_TYPES_H__
#define __TCPIP_TYPES_H__

#include <resea.h>
#include <resea/tcpip.h>

#define TCPIP_LITTLE_ENDIAN __ORDER_LITTLE_ENDIAN__
#define PACKED __attribute__((packed))

#define PORT_ANY       0
typedef uint16_t tcpip_port_t;

typedef uint16_t  net_type_t;
typedef uint8_t   trans_type_t;

// same as ethtype
enum net_types {
    NET_TYPE_ARP  = 0x0806,
    NET_TYPE_IPV4 = 0x0800
};

// same as protocol type in a IP header
enum trans_types {
    TRANS_TYPE_ICMP = 0x01,
    TRANS_TYPE_TCP  = 0x06,
    TRANS_TYPE_UDP  = 0x11,
};

struct addr {
    net_type_t protocol;
    uint16_t port;

    union {
        struct {
            uint32_t ipv4_addr;
        };
    };
};

#endif
