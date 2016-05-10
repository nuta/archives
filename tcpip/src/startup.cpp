#include <resea.h>
#include <resea/tcpip.h>
#include "socket.h"


extern "C" void tcpip_startup(void) {

    tcpip_init_socket();
}
