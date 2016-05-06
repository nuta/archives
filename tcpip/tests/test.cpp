#include <resea.h>
#include "../src/endian.h"


extern "C" void tcpip_test(void) {

    TEST_EXPECT(tcpip_to_host_endian16(0)          == tcpip_to_net_endian16(0));
    TEST_EXPECT(tcpip_to_host_endian16(0xaabb)     == tcpip_to_net_endian16(0xaabb));
    TEST_EXPECT(tcpip_to_host_endian32(0xaabb)     == tcpip_to_net_endian32(0xaabb));
    TEST_EXPECT(tcpip_to_host_endian32(0x12345678) == tcpip_to_net_endian32(0x12345678));
    TEST_END();
}
