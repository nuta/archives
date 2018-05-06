/* TODO: rewrite in Intel syntax */
#define ATT_SYNTAX
#include "asmdefs.h"
#include "msr.h"

.globl start_kernel_thread
start_kernel_thread:
    /* arch_create_thred() pushed `arg` and `start` onto the stack. */
    pop %rdi
    ret

.globl enter_userspace
enter_userspace:
    /* arch_create_thread() pushed to the kernel stack (current one)
      `arg' and an IRET frame. */

    pop %rdi /* arg */

    /* Sanitize registers except RDI to prevent information leak. */
    xor %rax, %rax
    xor %rbx, %rbx
    xor %rcx, %rcx
    xor %rdx, %rdx
    xor %rsi, %rsi
    xor %rbp, %rbp
    xor %r8, %r8
    xor %r9, %r9
    xor %r10, %r10
    xor %r11, %r11
    xor %r12, %r12
    xor %r13, %r13
    xor %r14, %r14
    xor %r15, %r15

    /* Current GS base is kernel one. Swap as the timer ISR does. */
    swapgs

    /* TODO: sanitize other registers */
    iretq

/* void arch_switch(struct arch_thread *prev, struct arch_thread *next); */
.globl arch_switch
arch_switch:
    /* TODO: save and restore FPU registers and debug registers. */

    /* Switch GS register. TODO: save GS register. */
    mov 32(%rsi), %rax
    mov %ax, %gs

    /* Set GS base addresses. Note that we're in kernel space; the
       current GS base will be swapped by SWAPGS in the interrupt
       handler. */
    lea 40(%rsi), %rdx
    mov $MSR_GS_BASE, %ecx
    mov %edx, %eax
    shrq $32, %rdx
    wrmsr

    /* We don't allow using GS register in userspace for now. */
    mov $MSR_KERNEL_GS_BASE, %ecx
    xor %eax, %eax
    xor %edx, %edx
    wrmsr

    /* We have to restore Interrupt Flags properly. */
    pushfq
    pop %rdx
    pushq 16(%rsi)      /* Restore next's RFLAGS. */
    popfq

    movabs $1f, %rax
    movq %rax, 0(%rdi)  /* Set prev RIP to label "1". */
    movq %rsp, 8(%rdi)  /* Save the current RSP to prev. */
    movq %rdx, 16(%rdi) /* Save the RFLAGS to prev. */
    movq 8(%rsi), %rsp  /* Restore next's RSP. */
    jmpq *0(%rsi)       /* Resume next. */

1:
    /* Resumed thread starts from here. Just return from arch_switch(). */
    ret
