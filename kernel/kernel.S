.intel_syntax noprefix
.code16

.globl test
test:
    lea si, [msg]
    call puts
    cli
    hlt

msg:
    .asciz "\nHello from real mode!\n"

puts:
    mov ah, 0x01    // Send a character feature.
    mov dx, 0x0000  // The COM port.
putchar:
    cmp si, 0
    je 1f
    mov al, [si]
    int 0x14
    inc si
    jmp putchar
1:
    ret