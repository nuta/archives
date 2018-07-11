#include <kernel/printk.h>
#include "asm.h"
#include "msr.h"
#include "pmc.h"

static bool pmc_enabled = false;


static inline u64_t rdpmc(u32_t reg) {
    u32_t low, high;
    __asm__ __volatile__("rdpmc" : "=a"(low), "=d"(high) : "c"(reg));
    return ((u64_t) high << 32) | low;
}
void x64_init_pmc(void) {
    // Ensure that perfotmance monitor features are supported.
    u32_t eax, ebx, ecx, edx;
    asm_cpuid(0x0a, &eax, &ebx, &ecx, &edx);
    int version = eax & 0xff;
    int counter_num = (eax >> 8) & 0xff;
    if (version < 1) {
        WARN("Performance monitor features are not supported.");
        return;
    }

    if (counter_num < 4) {
        WARN("# of PMC registers is less than 4.");
        return;
    }

    INFO("Performance monitor features: version=%d, counters=%d", version, counter_num);

    // Enable PMC0 - PMC2.
    asm_wrmsr(MSR_PERF_GLOBAL_CTRL, 7);
    pmc_enabled = true;

    // Allow reading PMC registers in user.
    // TODO: disable by default for security
    asm_set_cr4(asm_get_cr4() | (1 << 8));

    // Set up PMC0.
    asm_wrmsr(MSR_PERFEVTSEL(0), PMC_EVENT_UNHALTED_CORE_CYCLES | PMC_KERNEL | PMC_USER | PMC_ENABLE);
    asm_wrmsr(MSR_PERFEVTSEL(1), PMC_EVENT_MEM_LOAD_UOPS_RETIRED_LLC_MISS | PMC_KERNEL | PMC_USER | PMC_ENABLE);
}
