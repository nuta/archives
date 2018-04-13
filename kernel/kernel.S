.intel_syntax noprefix
.code16

.macro putchar ch
    mov ah, 0x01
    mov dx, 0x0000
    mov al, \ch
    int 0x14
.endm

.globl main
main:
    putchar '\n'
    putchar 'H'
    putchar 'e'
    putchar 'l'
    putchar 'l'
    putchar 'o'
    putchar ' '
    putchar 'w'
    putchar 'o'
    putchar 'r'
    putchar 'l'
    putchar 'd'
    putchar '!'
    putchar '!'
    putchar '!'
    hlt
