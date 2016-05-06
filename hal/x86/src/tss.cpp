#include <hal.h>


void x86_init_tss (struct TSS *tss) {

    CPUVAR->intr_stack_addr = 0xffff800000007c00; // FIXME
    tss->ist[INT_HANDLER_IST - 1] = CPUVAR->intr_stack_addr;
    tss->iomap  = sizeof(struct TSS);  // this disables I/O permission map
}
