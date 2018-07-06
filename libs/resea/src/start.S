.text
.intel_syntax noprefix

.extern main
.global start
start:
    mov rdi, 0
    mov rsi, 0
    call main
    ud2

.globl _Unwind_Resume
_Unwind_Resume:
    ud2

/* void *memcpy(void *dst, const void *src, size_t n); */
.globl memcpy
memcpy:
    mov rax, rdi
    xchg rcx, rdx
    cld
    rep movsb
    ret

/* void *memset(void *ptr, int value, size_t n); */
.globl memset
memset:
    mov r8, rdi
    xchg rcx, rdx
    mov al, sil
    cld
    rep stosb
    mov rax, r8
    ret

/* void *memcmp(void *ptr1, void *ptr2, size_t n); */
.globl memcmp
memcmp:
    xchg rcx, rdx
    cld
    rep cmpsb

    test rcx, 0
    jne 1f
    xor rax, rax
    ret

1:
    mov al, [rdi]
    sub al, [rsi]
    ret
