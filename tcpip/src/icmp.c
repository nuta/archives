#include "icmp.h"
#include "endian.h"
#include "printf.h"


void tcpip_receive_icmp(struct tcpip_instance *instance,
  struct tcpip_addr *src_addr, struct tcpip_addr *dest_addr,
  const void *payload, size_t size) {

    struct tcpip_icmp_header *header = (struct tcpip_icmp_header *) payload;
    uint8_t type, code, checksum;
    uint32_t data;
    char * type_str;

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

