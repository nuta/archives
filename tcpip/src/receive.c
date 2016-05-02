#include "receive.h"
#include "arp.h"
#include "ipv4.h"
#include "printf.h"


typedef void (*tcpip_receiver_t)(struct tcpip_instance *instance,
    const void *payload, size_t size);

static tcpip_receiver_t receivers[TCPIP_LINKTYPE_MAX] = {
    tcpip_receive_arp,
    tcpip_receive_ipv4,
};


void tcpip_receive_packet(struct tcpip_instance *instance, tcpip_link_type_t type,
    const void *payload, size_t size) {

    receivers[type](instance, payload, size);
}

