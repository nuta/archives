#include "tcpip.h"
#include "icmp.h"
#include "endian.h"
#include "printf.h"


void tcpip_receive_icmp(struct addr *src_addr, struct addr *dest_addr,
                        struct mbuf *mbuf) {

    struct tcpip_icmp_header *header;
    uint8_t type, code, checksum;
    uint32_t data;
    char const *type_str;

    header = (struct tcpip_icmp_header *) &mbuf->data[mbuf->begin];
    type = header->type;
    code = header->code;
    checksum = tcpip_to_host_endian16(header->checksum);
    data     = tcpip_to_host_endian32(header->data);
    
    switch (type) {
    case TCPIP_ICMPTYPE_ECHO_REPLY:   type_str = "ECHO REPLY "; break;
    case TCPIP_ICMPTYPE_ECHO_REQUEST: type_str = "ECHO REQUEST "; break;
    default: type_str = "";
    }

    DEBUG("%stype=%02x, code=%02x data=%04x",
      type_str, type, code, data);
}

