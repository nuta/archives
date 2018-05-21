.intel_syntax noprefix

/* channel_t ipc_open(void) */
.globl ipc_open
ipc_open:
    mov rax, 1
    syscall
    ret


/* void ipc_close(channel_t ch) */
.globl ipc_close
ipc_close:
    mov rax, 2
    syscall
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
    push r9
    push r8
    push rcx
    push rdx
    push rsi
    mov rax, 4
    syscall

    pop rdi
    mov [rdi], rdx
    pop rdi
    mov [rdi], r8
    pop rdi
    mov [rdi], r9
    pop rdi
    mov [rdi], r10
    pop rdi
    mov [rdi], rsi
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
    xchg rcx, r10
    mov rax, 5
    syscall

    mov rdi, [rsp + 8]
    mov [rdi], rdx
    mov rdi, [rsp + 16]
    mov [rdi], r8
    mov rdi, [rsp + 24]
    mov [rdi], r9
    mov rdi, [rsp + 32]
    mov [rdi], r10
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
    push rdi
    mov rdi, [rdi]
    xchg rcx, r10
    mov rax, 6
    syscall

    pop rdi
    mov [rdi], rdx
    mov rdx, [rsp + 8]
    mov [rdx], r8
    mov rdx, [rsp + 16]
    mov [rdx], r9
    mov rdx, [rsp + 24]
    mov [rdx], r10
    mov rdx, [rsp + 32]
    mov [rdx], rsi
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
    xchg bx,bx
    ret
