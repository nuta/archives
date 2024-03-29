.intel_syntax noprefix
.set INTERRUPT_HANDLER_SIZE, 16
.set GS_RSP0,                0
.set GS_RSP3,                8

.global syscall_entry
syscall_entry:
    // "A.2.1 Calling Conventions" in https://uclibc.org/docs/psABI-x86_64.pdf
    //
    //   RAX: syscall number
    //   RDI: arg1
    //   RSI: arg2
    //   RDX: arg3
    //   RCX: user RIP (set by SYSCALL)
    //   R8:  arg5
    //   R9:  arg6
    //   R10: arg4
    //   R11: user RFLAGS (set by SYSCALL)
    //   The others: user values (need to be preserved)

    // Interrupts are automatically disabled by MSR_FMASK. If interrupts are
    // enabled, an interrupt may occurs before SWAPGS and as a result, an
    // interrupt handler mistakenly use the user's GS base because the interrupt
    // has occurred in the kernel mode and SWAPGS won't be performed.
    swapgs

    // Switch to the kernel stack.
    mov gs:[GS_RSP3], rsp
    mov rsp, gs:[GS_RSP0]

    // Start accepting interrupts.
    // sti FIXME:

    // Save SYSRET context and registers onto the kernel stack.
    push gs:[GS_RSP3] // User RSP.
    push r11 // User RFLAGS.
    push rcx // User RIP.
    push rbp
    push rbx
    push rdx
    push rdi
    push rsi
    push r8
    push r9
    push r10
    push r12
    push r13
    push r14
    push r15

    mov rcx, r10 // arg3
    push rsp     // syscall frame
    push rax     // syscall number
    call x64_handle_syscall
    add rsp, 16

    // Restore registers.
    pop r15
    pop r14
    pop r13
    pop r12
    pop r10
    pop r9
    pop r8
    pop rsi
    pop rdi
    pop rdx
    pop rbx
    pop rbp
    pop rcx
    pop r11
    pop rsp

    // Ensure that interrupts are disabled since SWAPGS and SYSRETQ are not a
    // atomic operation. If an interrupt occurred between SWAPGS and SYSRETQ
    // (thus the GS base points to the user one), the interrupt handler won't
    // perform SWAPGS since the interrupt has occurred in kernel mode.
    cli
    swapgs
    sysretq

//
//  Interrupt/exception handlers
//
.align INTERRUPT_HANDLER_SIZE
.global interrupt_handlers
interrupt_handlers:
.set i, 0
.rept 256
.set handler_start, .
// Exceptions with error code.
.if i == 8 || 10 <= i && i <= 14 || i == 17
    .align INTERRUPT_HANDLER_SIZE
    cli
    push i
    jmp interrupt_common
    .align INTERRUPT_HANDLER_SIZE
// Interrupts and exceptions without error code.
.else
    .align INTERRUPT_HANDLER_SIZE
    cli
    push 0 // Dummy value as error code.
    push i
    jmp interrupt_common
    .align INTERRUPT_HANDLER_SIZE
.endif

// Increment the counter.
.set i, i + 1
.endr

.extern x64_handle_interrupt
interrupt_common:
    //
    //  The current stack frame:
    //
    //            +--------------------+
    //     48     |        SS          |
    //            +--------------------+
    //     40     |        RSP         |
    //            +--------------------+
    //     32     |       RFLAGS       |
    //            +--------------------+
    //     24     |        CS          |
    //            +--------------------+
    //     16     |        RIP         |
    //            +--------------------+
    //      8     |     Error code     |
    //            +--------------------+
    //      0     |     IRQ Number     | <- RSP
    //            +--------------------+
    //

    // Check CS register in the IRET frame to determine if the interrupt has
    // occurred in user mode.
    test qword ptr [rsp + 24], 3
    jz 1f
    swapgs
1:
    // Save RDI and set the IRQ number to RDI at once.
    xchg rdi, [rsp]

    // Save registers except RDI (we have already saved it above).
    push r15
    push r14
    push r13
    push r12
    push r11
    push r10
    push r9
    push r8
    push rbp
    push rsi
    push rdx
    push rcx
    push rbx
    push rax

    mov rsi, rsp
    call x64_handle_interrupt

    pop rax
    pop rbx
    pop rcx
    pop rdx
    pop rsi
    pop rbp
    pop r8
    pop r9
    pop r10
    pop r11
    pop r12
    pop r13
    pop r14
    pop r15
    pop rdi

    // Skip error code.
    add rsp, 8

    // Check CS register in the IRET frame to determine whether the exception
    // occur in the userspace. If so, do SWAPGS.
    test qword ptr [rsp + 8], 3
    jz 1f

    cli
    swapgs
1:
    iretq

.global kthread_entry
kthread_entry:
    // Release held locks before enabling interrupts.
    call after_switch

    sti

    // Clear the frame pointer to stop backtracing here.
    xor rbp, rbp

    // Pop the entry point and start executing.
    pop rax
    call rax

    // Unreachable.
    ud2

.global userland_entry
userland_entry:
    // Release held locks before enabling interrupts.
    call after_switch

    // Sanitize registers to prevent information leak.
    xor rax, rax
    xor rbx, rbx
    xor rcx, rcx
    xor rdx, rdx
    xor rdi, rdi
    xor rsi, rsi
    xor r8, r8
    xor r9, r9
    xor r10, r10
    xor r11, r11
    xor r12, r12
    xor r13, r13
    xor r14, r14
    xor r15, r15
    xor rbp, rbp

    cli
    swapgs
    iretq

.global forked_child_entry
forked_child_entry:
    call after_switch

    pop rdx
    pop rdi
    pop rsi
    pop r8
    pop r9
    pop r10

    // Restore SYSCALL-related registers.
    pop rcx
    pop r11

    // Set fork(2)'s the return value.
    xor rax, rax

    cli
    swapgs
    iretq

/// fn do_switch_thread(prev_rsp: u64, next_rsp: u64);
///
/// Saves the current context into `prev` and restore the context from `next`.
///
/// A *context* is a state of registers including:
///
///   - Instruction Pointer (implicitly saved and restored by CALL/RET)
///   - Stack Pointer (RSP)
///   - callee-saved registers (RBP, RBX, R12-R15)
///   - RFLAGS
///
.global do_switch_thread
do_switch_thread:
    /* Save callee-saved registers. */
    push rbp
    push rbx
    push r12
    push r13
    push r14
    push r15
    pushfq

    /* Save and restore the stack pointer. */
    mov [rdi], rsp
    mov rsp, [rsi]

    /* Restore callee-saved registers. */
    popfq
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbx
    pop rbp

    /* Resume the next thread. */
    ret
