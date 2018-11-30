#include <kernel/printk.h>
#include "asm.h"
#include "fpu.h"

u64_t x64_xsave_mask;

void x64_init_fpu(void) {
    // Ensure that XSAVE is supported.
    u32_t eax, ebx, ecx, edx;
    asm_cpuid(0x01, &eax, &ebx, &ecx, &edx);
    if (!(ecx & (1 << 26))) {
        PANIC("XSAVE is not supported in this processor.");
    }

    // Set CR4.OSXSAVE.
    asm_set_cr4(asm_get_cr4() | (1 << 18));

    // Enable x87 and SSE.
    // TODO: Do CPUID to ensure that they are supported.
    x64_xsave_mask = 1 | (1 << 1);

    // set XCR0.
    asm_set_xcr(0, x64_xsave_mask);

    // Ensure that arch_thread.xstate is large enough to xsave.
    u32_t xstate_size;
    ecx = 0;
    asm_cpuid(0x0d, &eax, &xstate_size, &ecx, &edx);
    if (xstate_size > XSTATE_SIZE) {
        PANIC("arch_thread.xstate is too short (cpuid returned %d).", xstate_size);
    }
}
