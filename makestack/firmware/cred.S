.section .rodata
.align 8
.globl __cred_header, __cred
__cred_header:
    .ascii "__MAKESTACK_CRED_START__"
__cred:
    .space 1024
    .ascii "__MAKESTACK_CRED_END__"
