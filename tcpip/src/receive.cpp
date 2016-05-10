#include "receive.h"
#include "arp.h"
#include "ipv4.h"
#include "printf.h"


typedef void (*tcpip_receiver_t)(const void *payload, size_t size);

static tcpip_receiver_t receivers[TCPIP_LINKTYPE_MAX] = {
    tcpip_receive_arp,
    tcpip_receive_ipv4,
};


void tcpip_receive_packet(tcpip_link_type_t type, const void *payload, size_t size) {

    receivers[type](payload, size);
}

