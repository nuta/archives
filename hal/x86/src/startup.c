#include <hal.h>


handler_t x86_handler;
static channel_t ch;

void hal_startup(void) {

    sys_open(&ch);
    sys_setoptions(ch, &x86_handler, NULL, 0);
}
