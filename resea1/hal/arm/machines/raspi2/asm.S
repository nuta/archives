// Unset IRQ mask bit
.global arm_accept_irq
arm_accept_irq:
    mrs  r0, cpsr
    bic  r0, r0, #0x80
    msr  cpsr_c, r0
    bx   lr


// Restore a thread state.
.global arm_asm_resume_thread
arm_asm_resume_thread:
    // r0 = sp
    // r1 = spsr

    mov    sp, r0
    msr    spsr, r1
    ldmfd  sp!, {r0-r12, lr, pc}^
