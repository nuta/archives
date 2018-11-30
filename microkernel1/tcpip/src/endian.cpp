#include "endian.h"

namespace tcpip {

static uint16_t swap16(uint16_t x) {
    return ((x & 0xff00) >> 8) | ((x & 0x00ff) << 8);
}


static uint32_t swap32(uint32_t x) {
    return ((x & 0xff000000) >> 24) |
           ((x & 0x00ff0000) >> 8)  |
           ((x & 0x0000ff00) << 8)  |
           ((x & 0x000000ff) << 24);
}


uint16_t to_host_endian16(uint16_t x) {
#ifdef TCPIP_LITTLE_ENDIAN
    return swap16(x);
#else
    return x;
#endif
}


uint32_t to_host_endian32(uint32_t x) {
#ifdef TCPIP_LITTLE_ENDIAN
    return swap32(x);
#else
    return x;
#endif
}


uint16_t to_net_endian16(uint16_t x) {
#ifdef TCPIP_LITTLE_ENDIAN
    return swap16(x);
#else
    return x;
#endif
}


uint32_t to_net_endian32(uint32_t x) {
#ifdef TCPIP_LITTLE_ENDIAN
    return swap32(x);
#else
    return x;
#endif
}

} // namespace tcpip
