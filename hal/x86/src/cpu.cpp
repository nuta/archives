#include "_x86.h"
#include <hal.h>

void hal_panic(void) {

    x86_asm_cli();

    for(;;)
        x86_asm_hlt();
}
