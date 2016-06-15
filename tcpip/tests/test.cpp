#include "../src/tcpip.h"
#include "../src/endian.h"
#include <resea.h>
#include <resea/channel.h>
#include <resea/tcpip.h>
#include <resea/net_device.h>
#include <string.h>

using namespace tcpip;
static channel_t tcpip_ch;


static void net_device_mock_server(channel_t ch, payload_t *m) {

    switch (EXTRACT_MSGID(m)) {
    default:
        BUG("net_device_mock: unsupported msg");
    }
}


static void udp_server(channel_t ch, payload_t *m) {
    ident_t socket;
    uint16_t port;
    void *data;
    result_t r;

    switch (EXTRACT_MSGID(m)) {
    case MSGID(tcpip, received):
        socket = EXTRACT(m, tcpip, received, socket);
        port   = EXTRACT(m, tcpip, received, port);
        data   = EXTRACT(m, tcpip, received, payload);
        DEBUG("udp_server: received '%s'", data);
        TEST_EXPECT(strcmp((const char *) data, "HELO") == 0);

        call_tcpip_sendto(tcpip_ch, socket, TCPIP_PROTOCOL_IPV4,
          (void *) "", 0, port,
          (void *) "HOWDY", 6, &r);
        break;
    }
}


static void udp_client(channel_t ch, payload_t *m) {
    void *data;

    switch (EXTRACT_MSGID(m)) {
    case MSGID(tcpip, received):
        data = EXTRACT(m, tcpip, received, payload);
        DEBUG("udp_client: received '%s'", data);
        TEST_EXPECT(strcmp((const char *) data, "HOWDY") == 0);
        TEST_END();
        break;
    }
}


extern "C" void tcpip_test() {
    channel_t mock_ch;
    result_t r;

    // launch a mock
    mock_ch = create_channel();
    call_channel_register(connect_to_local(1), mock_ch,
        INTERFACE(net_device), &r);
    set_channel_handler(mock_ch, net_device_mock_server);

    // connect to tcpip
    tcpip_ch = create_channel();
    call_channel_connect(connect_to_local(1), tcpip_ch, INTERFACE(tcpip), &r);

    //
    //  Endian conversions
    //
    TEST_EXPECT(to_host_endian16(0)          == to_net_endian16(0));
    TEST_EXPECT(to_host_endian16(0xaabb)     == to_net_endian16(0xaabb));
    TEST_EXPECT(to_host_endian32(0xaabb)     == to_net_endian32(0xaabb));
    TEST_EXPECT(to_host_endian32(0x12345678) == to_net_endian32(0x12345678));

    //
    //  UDP
    //
    char const *localhost = "127.0.0.1";
    uint8_t const data[] = {'H', 'E', 'L', 'O', '\0'};
    ident_t server_sock, client_sock;
    channel_t udp_server_ch, udp_client_ch;

    udp_server_ch = create_channel();
    udp_client_ch = create_channel();
    set_channel_handler(udp_server_ch, udp_server);
    set_channel_handler(udp_client_ch, udp_client);

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
