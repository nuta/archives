.intel_syntax noprefix

/* void arch_copy_from_user(void *kernel, uptr_t user, size_t length); */
.globl arch_copy_from_user
.globl x64_do_copy_from_user
arch_copy_from_user:
    cld
    xchg rdx, rcx

/* A page fault in MOVSB is treated as an invalid access by user. */
x64_do_copy_from_user:
    rep movsb

    ret
