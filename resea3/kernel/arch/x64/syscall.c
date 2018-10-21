#include <kernel/types.h>
#include "syscall.h"
#include "gdt.h"
#include "msr.h"
#include "asm.h"
#include "handler.h"

void x64_init_syscall(void) {
    /*
     * In 64-bit mode CPU adds 16 to user CS segment specified in STAR so
     * we specify USER_CODE32_SEG instead of USER_CODE_SEG.
     */
    STATIC_ASSERT(USER_CODE32_SEG + 16 == USER_CODE64_SEG, "SYSRET constraint");

    asm_wrmsr(MSR_STAR, ((u64_t) USER_CODE32_SEG << 48) | ((u64_t) KERNEL_CODE64_SEG << 32));
    asm_wrmsr(MSR_LSTAR, (uptr_t) x64_syscall_handler);

    // RIP for compatibility mode. We don't support it for now.
    asm_wrmsr(MSR_CSTAR, 0);
    asm_wrmsr(MSR_SFMASK, 0);

    // Enable SYSCALL/SYSRET.
    asm_wrmsr(MSR_EFER, asm_rdmsr(MSR_EFER) | EFER_SCE);
}
