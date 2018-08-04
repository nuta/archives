.intel_syntax noprefix

.extern main
.globl start
start:
    call main

    /* Terminate the current process. */
    mov ah, 0x4c
    syscall

    /* Failed to terminate. Execute an invalid opcode instead. */
    ud2
