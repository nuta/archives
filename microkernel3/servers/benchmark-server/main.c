#include <resea.h>
#include <resea/benchmark.h>
#include <resea/discovery.h>

void main(void) {
    channel_t server = ipc_open();
    call_discovery_register(1, BENCHMARK_SERVICE, server);

    payload_t a0, a1, a2, a3;
    channel_t from;
    ipc_recv(server, &from, &a0, &a1, &a2, &a3);
    for (;;) {
        ipc_replyrecv(&from, 0, 0, 0, 0, 0, &a0, &a1, &a2, &a3);
    }
}
