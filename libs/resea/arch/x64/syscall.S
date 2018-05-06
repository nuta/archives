.intel_syntax noprefix

/* channel_t ipc_open(void) */
.globl ipc_open
ipc_open:
    push r14
    push r13
    push r12

    mov r14, 1
    syscall

    pop r12
    pop r13
    pop r14
    ret


/*
 *  header_t ipc_send(
 *      channel_t ch,
 *      header_t type,
 *      payload_t a0,
 *      payload_t a1,
 *      payload_t a2,
 *      payload_t a3
 *  );
 */
.globl ipc_send
ipc_send:
    ret

/*
 *  header_t ipc_recv(
 *      channel_t ch,
 *      channel_t *from,
 *      payload_t *a0,
 *      payload_t *a1,
 *      payload_t *a2,
 *      payload_t *a3
 *  );
 */
 .globl ipc_recv
ipc_recv:
    push rbp
    mov rbp, rsp
    push r14
    push r13
    push r12

    push rsi
    push rdx
    push rcx
    push r8
    push r9

    xchg rcx, r12
    mov r14, 4
    syscall

    pop r14
    pop r11
    pop rcx
    pop rdx
    pop rsi
    mov [rsi], r8
    mov [rdx], r9
    mov [rcx], r10
    mov [r11], r12
    mov [r14], r13

    pop r12
    pop r13
    pop r14
    pop rbp
    ret

/*
 *  header_t ipc_call(
 *      channel_t ch,
 *      header_t type,
 *      payload_t a0,
 *      payload_t a1,
 *      payload_t a2,
 *      payload_t a3,
 *      payload_t *r0,
 *      payload_t *r1,
 *      payload_t *r2,
 *      payload_t *r3
 *  );
 */
.globl ipc_call
ipc_call:
    push rbp
    mov rbp, rsp
    push r14
    push r13
    push r12

    xchg rcx, r12
    mov r14, 5
    syscall

    mov r14, [rbp + 16]
    mov [r14], r8
    mov r14, [rbp + 24]
    mov [r14], r9
    mov r14, [rbp + 32]
    mov [r14], r10
    mov r14, [rbp + 40]
    mov [r14], r12

    pop r12
    pop r13
    pop r14
    pop rbp
    ret

/*
 *  header_t ipc_replyrecv(
 *      channel_t *from,
 *      header_t type,
 *      payload_t r0,
 *      payload_t r1,
 *      payload_t r2,
 *      payload_t r3,
 *      payload_t *a0,
 *      payload_t *a1,
 *      payload_t *a2,
 *      payload_t *a3
 *
 */
 .globl ipc_replyrecv
ipc_replyrecv:
    push rbp
    mov rbp, rsp
    push r14
    push r13
    push r12
    push rdi

    mov rdi, [rdi]
    xchg rcx, r12
    mov r14, 6
    syscall

    pop rdi
    mov [rdi], r8
    mov r14, [rbp + 16]
    mov [r14], r9
    mov r14, [rbp + 24]
    mov [r14], r10
    mov r14, [rbp + 32]
    mov [r14], r12
    mov r14, [rbp + 40]
    mov [r14], r13

    pop r12
    pop r13
    pop r14
    pop rbp
    ret

/* header_t ipc_discard(payload_t ool0, payload_t ool1, payload_t ool2, payload_t ool3); */
.globl ipc_discard
ipc_discard:
    ret

/* header_t ipc_link(channel_t ch0, channel_t ch1); */
.globl ipc_link
ipc_link:
    ret

/* header_t ipc_transfer(channel_t ch, channel_t dest); */
.globl ipc_transfer
ipc_transfer:
    ret

/* channel_t ipc_connect(channel_t server); */
.globl ipc_connect
ipc_connect:
    push r13
    push r14
    push r12

    mov r14, 9
    syscall

    pop r12
    pop r14
    pop r13
    ret
