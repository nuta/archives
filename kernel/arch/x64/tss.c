#include <kernel/string.h>
#include <kernel/memory.h>
#include "asm.h"
#include "cpu.h"
#include "tss.h"
#include "gdt.h"

void x64_init_tss(void) {
    struct tss *tss = &CPUVAR->tss;
    void *stack = kmalloc(INTR_STACK_SIZE, KMALLOC_NORMAL);

    memset(&CPUVAR->tss, 0, sizeof(CPUVAR->tss));

    // Interrupt/Exception kernel stack.
    tss->ist[INTR_HANDLER_IST - 1] = ((uptr_t) stack) + INTR_STACK_SIZE;

    // Disable I/O permission map.
    tss->iomap = sizeof(struct tss);

    asm_ltr(GDT_TSS_SEG);
}
