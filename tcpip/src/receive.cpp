#include <string.h>
#include "receive.h"
#include "arp.h"
#include "mbuf.h"
#include "ipv4.h"
#include "printf.h"

using namespace tcpip;

namespace tcpip {

void receive_packet(net_type_t type, const void *payload, size_t size) {
    struct mbuf *m;

    m = pack_mbuf(payload, size);

    switch (type) {
    case NET_TYPE_ARP:  receive_arp(m);  break;
    case NET_TYPE_IPV4: receive_ipv4(m); break;
    }
}

} // namespace tcpip
