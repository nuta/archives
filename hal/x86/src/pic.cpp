#include "_x86.h"
#include <hal.h>


void x86_disable_pic (void) {

    /* initialize before disabling not to receive interrupts */
    x86_asm_out8(0xa1, 0xff);
    x86_asm_out8(0x21, 0xff);

    x86_asm_out8(0x20, 0x11);
    x86_asm_out8(0xa0, 0x11);
    x86_asm_out8(0x21, 0x20);
    x86_asm_out8(0xa1, 0x28);
    x86_asm_out8(0x21, 0x04);
    x86_asm_out8(0xa1, 0x02);
    x86_asm_out8(0x21, 0x01);
    x86_asm_out8(0xa1, 0x01);

    x86_asm_out8(0xa1, 0xff);
    x86_asm_out8(0x21, 0xff);
}
