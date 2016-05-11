#include <resea.h>
#include <resea/channel.h>
#include <resea/tcpip.h>
#include <string.h>
#include "../src/endian.h"

static channel_t tcpip_ch;

static void udp_server(channel_t ch, payload_t *m) {
    ident_t socket;
    uint16_t port;
    void *data;
    result_t r;

    switch (m[1]) {
    case MSGID(tcpip, received):
        socket = EXTRACT(m, tcpip, received, socket);
        port   = EXTRACT(m, tcpip, received, port);
        data   = EXTRACT(m, tcpip, received, payload);
        DEBUG("udp_server: received '%s'", data);

        call_tcpip_sendto(tcpip_ch, socket, TCPIP_PROTOCOL_IPV4,
          (void *) "", 0, port,
          (void *) "HOWDY", 6, &r);
        break;
    }
}


static void udp_client(channel_t ch, payload_t *m) {
    void *data;

    switch (m[1]) {
    case MSGID(tcpip, received):
        data = EXTRACT(m, tcpip, received, payload);
        DEBUG("udp_client: received '%s'", data);
        if (strcmp((const char *) data, "HOWDY") == 0)
            TEST_END();
        break;
    }
}


extern "C" void tcpip_test(void) {
    result_t r;

    tcpip_ch = sys_open();
    call_channel_connect(connect_to_local(1), tcpip_ch, INTERFACE(tcpip), &r);

    //
    //  Endian conversions
    //
    TEST_EXPECT(tcpip_to_host_endian16(0)          == tcpip_to_net_endian16(0));
    TEST_EXPECT(tcpip_to_host_endian16(0xaabb)     == tcpip_to_net_endian16(0xaabb));
    TEST_EXPECT(tcpip_to_host_endian32(0xaabb)     == tcpip_to_net_endian32(0xaabb));
    TEST_EXPECT(tcpip_to_host_endian32(0x12345678) == tcpip_to_net_endian32(0x12345678));

    //
    //  UDP
    //
    char const *localhost = "127.0.0.1";
    uint8_t const data[] = {'H', 'E', 'L', 'O', '\0'};
    ident_t server_sock, client_sock;
    channel_t udp_server_ch, udp_client_ch;

    udp_server_ch = sys_open();
    udp_client_ch = sys_open();
    sys_setoptions(udp_server_ch, udp_server, nullptr, 0);
    sys_setoptions(udp_client_ch, udp_client, nullptr, 0);

    call_tcpip_open(tcpip_ch, TCPIP_PROTOCOL_UDP, udp_server_ch, &r, &server_sock);
    call_tcpip_open(tcpip_ch, TCPIP_PROTOCOL_UDP, udp_client_ch, &r, &client_sock);

    call_tcpip_bind(tcpip_ch, server_sock, TCPIP_PROTOCOL_IPV4,
      &localhost, strlen(localhost), 10000, &r);
    call_tcpip_bind(tcpip_ch, client_sock, TCPIP_PROTOCOL_IPV4,
      &localhost, strlen(localhost), 10001, &r);

    call_tcpip_sendto(tcpip_ch, client_sock, TCPIP_PROTOCOL_IPV4,
      &localhost, strlen(localhost), 10000,
      (void *) data, sizeof(data), &r);

    for(;;);
}
