#include <resea.h>

void tcpip_tcpip_open(channel_t __ch, tcpip_protocol_t a0, channel_t a1);
void tcpip_tcpip_close(channel_t __ch, ident_t a0);
void tcpip_tcpip_bind(channel_t __ch, ident_t a0, tcpip_protocol_t a1, void * a2, size_t a3, uint16_t a4);
void tcpip_tcpip_sendto(channel_t __ch, ident_t a0, tcpip_protocol_t a1, void * a2, size_t a3, uint16_t a4, void * a5, size_t a6);
