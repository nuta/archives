#include <string.h>
#include "receive.h"
#include "arp.h"
#include "mbuf.h"
#include "ipv4.h"
#include "printf.h"


typedef void (*tcpip_receiver_t)(struct mbuf *mbuf);

static tcpip_receiver_t receivers[LINKTYPE_MAX] = {
    tcpip_receive_arp,
    tcpip_receive_ipv4,
};


void tcpip_receive_packet(tcpip_link_type_t type, const void *payload, size_t size) {

    receivers[type](tcpip_pack_mbuf(payload, size));
}

