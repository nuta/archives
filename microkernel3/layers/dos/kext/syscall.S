#include <kernel/arch/x64/arch_thread.h>

.intel_syntax noprefix
.extern dos_syscall
.globl dos_syscall_handler
dos_syscall_handler:
    mov r8, rsp
    mov rsp, gs:[GS_RSP0]
    push r8
    push r11
    push rcx
    xchg rcx, r9

    sub rsp, 8 * 4
    mov [rsp + 0], rax
    mov [rsp + 8], rbx
    mov [rsp + 16], rcx
    mov [rsp + 24], rdx
    lea rdi, [rsp + 0]
    lea rsi, [rsp + 8]
    lea rdx, [rsp + 16]
    lea rcx, [rsp + 24]

    call dos_syscall

    mov rax, [rsp + 0]
    mov rbx, [rsp + 8]
    mov r9, [rsp + 16]
    mov rdx, [rsp + 24]
    add rsp, 8 * 4

    pop rcx
    pop r11
    pop rsp
    or  r11, gs:[GS_RFLAGS_ORMASK]
    swapgs
    sysret
