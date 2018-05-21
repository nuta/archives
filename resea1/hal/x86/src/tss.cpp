#include "x86.h"
#include <hal.h>
#include <resea/cpp/memory.h>


void x86_init_tss (struct TSS *tss) {
    uintptr_t v;
    paddr_t p;

    call_hal_callback(HAL_CALLBACK_ALLOCATE_MEMORY,
        0, INTERRUPT_STACK_SIZE, resea::interfaces::memory::ALLOC_WIRED,
        &v, &p);

    CPUVAR->intr_stack_addr = (uintptr_t) v + INTERRUPT_STACK_SIZE;
    tss->ist[INT_HANDLER_IST - 1] = CPUVAR->intr_stack_addr;
    tss->iomap  = sizeof(struct TSS);  // this disables I/O permission map
}
