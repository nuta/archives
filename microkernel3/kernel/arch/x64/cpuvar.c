#include <kernel/memory.h>
#include <kernel/printk.h>
#include "cpuvar.h"
#include "asm.h"

struct cpuvar *cpuvars[CPU_NUM_MAX];

void x64_init_cpuvar(void) {
    if (x64_get_processor_id() >= CPU_NUM_MAX) {
        PANIC("too many cpus");
    }

    cpuvars[x64_get_processor_id()] = kmalloc(sizeof(struct cpuvar), KMALLOC_NORMAL);
}
