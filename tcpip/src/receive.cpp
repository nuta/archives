#include <string.h>
#include "receive.h"
#include "arp.h"
#include "mbuf.h"
#include "ipv4.h"
#include "printf.h"


void tcpip_receive_packet(net_type_t type, const void *payload, size_t size) {
    struct mbuf *m;

    m = tcpip_pack_mbuf(payload, size);

    switch (type) {
    case NET_TYPE_ARP:  tcpip_receive_arp(m);  break;
    case NET_TYPE_IPV4: tcpip_receive_ipv4(m); break;
    }
}

