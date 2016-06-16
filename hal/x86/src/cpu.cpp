#include <hal.h>
#include "x86.h"


void hal_panic(void) {

    x86_asm_cli();

    for(;;)
        x86_asm_hlt();
}
