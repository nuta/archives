#include "asmdefs.h"
#include "arch_thread.h"

.macro SAVE_REGS
    push r15
    push r14
    push r13
    push r12
    push r11
    push r10
    push r9
    push r8
    push rbp
    push rdi
    push rsi
    push rdx
    push rcx
    push rbx
    push rax
.endm

.macro RESTORE_REGS
    pop rax
    pop rbx
    pop rcx
    pop rdx
    pop rsi
    pop rdi
    pop rbp
    pop r8
    pop r9
    pop r10
    pop r11
    pop r12
    pop r13
    pop r14
    pop r15
.endm

syscall_table:
    .quad invalid_syscall_handler /* 0 */
    .quad ipc_open_handler        /* 1 */
    .quad ipc_close_handler       /* 2 */
    .quad ipc_send_handler        /* 3 */
    .quad ipc_recv_handler        /* 4 */
    .quad ipc_call_handler        /* 5 */
    .quad ipc_replyrecv_handler   /* 6 */
    .quad invalid_syscall_handler /* 7 */
    .quad ipc_transfer_handler    /* 8 */
    .quad invalid_syscall_handler /* 9 */
    .quad invalid_syscall_handler /* 11 */
    .quad invalid_syscall_handler /* 12 */
    .quad invalid_syscall_handler /* 13 */
    .quad invalid_syscall_handler /* 14 */
    .quad invalid_syscall_handler /* 15 */

syscall_table_addr:
    .quad syscall_table

.extern handle_syscall
.globl x64_syscall_handler
x64_syscall_handler:
    swapgs
    cmp qword ptr gs:[GS_SYSCALL_HANDLER], 0
    je 1f
    jmp gs:[GS_SYSCALL_HANDLER]
1:
    /*
     *  RAX: syscall number
     *  RDI: 1st argument
     *  RSI: 2nd argument
     *  RDX: 3rd argument
     *  RCX: user RIP
     *  R8:  5th arugment
     *  R9:  6th arugment
     *  R10: 4th arugment
     *  R11: user RFLAGS
     *
     */
    and rax, 0x0f
    shl rax, 3
    add rax, [rip + syscall_table_addr]
    jmp [rax]

.macro ENTER_NORMAL_SYSCALL
    /* Switch to the kernel stack. */
    mov rax, rsp
    mov rsp, gs:[GS_RSP0]
    push rax /* user RSP */
    push r11 /* user RFLAGS */
    push rcx /* user RIP */
    xchg rcx, r10
.endm

.macro LEAVE_NORMAL_SYSCALL
    jmp return_from_syscall
.endm

.macro ENTER_RECV_SYSCALL
    /* Switch to the kernel stack. */
    mov rax, rsp
    mov rsp, gs:[GS_RSP0]
    push rax /* user RSP */
    push r11 /* user RFLAGS */
    push rcx /* user RIP */
    xchg rcx, r10
.endm

.macro LEAVE_RECV_SYSCALL
    cmp rax, 0x1000 /* NULL_PAGE_SIZE */
    jb 1f
    mov rdx, [rax + 8]
    mov r8,  [rax + 16]
    mov r9,  [rax + 24]
    mov r10, [rax + 32]
    mov rsi, [rax + 40]
    mov rax, [rax]
1:
    jmp return_from_syscall
.endm

ipc_open_handler:
    ENTER_NORMAL_SYSCALL
    call sys_open
    LEAVE_NORMAL_SYSCALL

ipc_close_handler:
    ENTER_NORMAL_SYSCALL
    call sys_close
    LEAVE_NORMAL_SYSCALL

ipc_send_handler:
    ENTER_NORMAL_SYSCALL
    call sys_send
    LEAVE_NORMAL_SYSCALL

ipc_transfer_handler:
    ENTER_NORMAL_SYSCALL
    call sys_transfer
    LEAVE_NORMAL_SYSCALL

ipc_recv_handler:
    /* Switch to the kernel stack. */
    ENTER_RECV_SYSCALL
    call sys_recv
    LEAVE_RECV_SYSCALL
1:
    jmp return_from_syscall

ipc_call_handler:
    ENTER_RECV_SYSCALL
    call sys_call
    cmp rax, 0x1000 /* NULL_PAGE_SIZE */
    jb 1f
    mov rdx, [rax + 16]
    mov r8,  [rax + 24]
    mov r9,  [rax + 32]
    mov r10, [rax + 40]
    mov rsi, [rax + 48]
    mov rax, [rax]
1:
    jmp return_from_syscall
/*    LEAVE_RECV_SYSCALL */

ipc_replyrecv_handler:
    ENTER_RECV_SYSCALL
    call sys_replyrecv
    LEAVE_RECV_SYSCALL

invalid_syscall_handler:
    mov rax, 1 /* ERROR_INVALID_SYSCALL */
    sysret

return_from_syscall:
    /*
     * RAX: message header
     * RDX: 1st return value
     * R8:  2nd return value
     * R9:  3rd return value
     * R10: 4th return value
     * RSI: 5th return value
     */

    pop rcx
    pop r11
    pop rsp
    or  r11, gs:[GS_RFLAGS_ORMASK]
    swapgs
    sysret

.extern x64_handle_irq
.macro INTR_HANDLER num
.align 8
.globl x64_irq_handler\num
x64_irq_handler\num:
    /*
     * The current RSP points to IST0, a shared interrupt handler stack.
     * Switch into the thread's own kernel stack since x64_handler_irq()
     * returns after context switches to other threads.
     */

    cli
    push rsi
    mov rsi, rsp

    /* Check CS register in the IRET frame. */
    test qword ptr [rsi + 16], 3
    jz 1f

    /* from user: use dedicated kernel stack */
    swapgs

    mov rsp, gs:[GS_RSP0] /* Extract RSP from gsinfo. */
    jmp 2f
1:
    /* from kernel: use current stack (assuming Red Zone is disabled) */
    mov rsp, [rsp + 32] /* Extract RSP from the IRET frame */
    jmp 2f

2:
    /* Copy IRET frame. */
    push [rsi + 40]    /* SS */
    push [rsi + 32]    /* RSP */
    push [rsi + 24]    /* RFLAGS */
    push [rsi + 16]    /* CS */
    push [rsi + 8]     /* RIP */
    mov rsi, [rsi + 0] /* Restore RSI. */
    SAVE_REGS

    mov rdi, \num
    call x64_handle_irq
    jmp ret_from_irq
.endm

ret_from_irq:
    RESTORE_REGS

    /* Check CS register in the IRET frame. */
    test qword ptr [rsp + 8], 3
    jz 1f
    swapgs
1:
    iretq


.extern x64_handle_exception
.macro EXP_HANDLER_WITH_ERRCODE num
.align 8
.globl  x64_exp_handler\num
x64_exp_handler\num:
    cli

    /* Get error code from the stack and overwrite by SAVE_REGS. */
    push rax
    mov  rax, [rsp + 8] /* error code */
    mov  [rsp - 112], rax
    mov  rax, [rsp + 16] /* RIP */
    mov  [rsp - 120], rax
    pop  rax
    add  rsp, 8 /* overwrite the error code */

    SAVE_REGS

    mov rdi, \num
    mov rsi, [rsp - 8] /* error code */
    mov rdx, [rsp - 16] /* RIP */
    call x64_handle_exception
    jmp ret_from_exp
.endm

ret_from_exp:
    RESTORE_REGS
    iretq

.macro EXP_HANDLER_WITHOUT_ERRCODE num
.align 8
.globl x64_exp_handler\num
x64_exp_handler\num:
    cli
    SAVE_REGS

    mov  rdi, \num
    call x64_handle_exception

    RESTORE_REGS
    iretq
.endm

.extern x64_handle_unkown_irq
.globl x64_unknown_irq_handler
.align 8
x64_unknown_irq_handler:
    cli
    SAVE_REGS

    call x64_handle_unkown_irq

    RESTORE_REGS
    iretq

EXP_HANDLER_WITHOUT_ERRCODE 0
EXP_HANDLER_WITHOUT_ERRCODE 1
EXP_HANDLER_WITHOUT_ERRCODE 2
EXP_HANDLER_WITHOUT_ERRCODE 3
EXP_HANDLER_WITHOUT_ERRCODE 4
EXP_HANDLER_WITHOUT_ERRCODE 5
EXP_HANDLER_WITHOUT_ERRCODE 6
EXP_HANDLER_WITHOUT_ERRCODE 7
EXP_HANDLER_WITH_ERRCODE    8
EXP_HANDLER_WITHOUT_ERRCODE 9
EXP_HANDLER_WITH_ERRCODE    10
EXP_HANDLER_WITH_ERRCODE    11
EXP_HANDLER_WITH_ERRCODE    12
EXP_HANDLER_WITH_ERRCODE    13
EXP_HANDLER_WITH_ERRCODE    14
EXP_HANDLER_WITH_ERRCODE    15
EXP_HANDLER_WITHOUT_ERRCODE 16
EXP_HANDLER_WITH_ERRCODE    17
EXP_HANDLER_WITHOUT_ERRCODE 18
EXP_HANDLER_WITHOUT_ERRCODE 19
EXP_HANDLER_WITHOUT_ERRCODE 20

INTR_HANDLER 0x20
INTR_HANDLER 0x21
INTR_HANDLER 0x22
INTR_HANDLER 0x23
INTR_HANDLER 0x24
INTR_HANDLER 0x25
INTR_HANDLER 0x26
INTR_HANDLER 0x27
INTR_HANDLER 0x28
INTR_HANDLER 0x29
INTR_HANDLER 0x2a
INTR_HANDLER 0x2b
INTR_HANDLER 0x2c
INTR_HANDLER 0x2d
INTR_HANDLER 0x2e
INTR_HANDLER 0x2f
INTR_HANDLER 0x30
INTR_HANDLER 0x31
INTR_HANDLER 0x32
INTR_HANDLER 0x33
INTR_HANDLER 0x34
INTR_HANDLER 0x35
INTR_HANDLER 0x36
INTR_HANDLER 0x37
INTR_HANDLER 0x38
INTR_HANDLER 0x39
INTR_HANDLER 0x3a
INTR_HANDLER 0x3b
INTR_HANDLER 0x3c
INTR_HANDLER 0x3d
INTR_HANDLER 0x3e
INTR_HANDLER 0x3f
INTR_HANDLER 0x40
