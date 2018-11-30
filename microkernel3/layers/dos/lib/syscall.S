.intel_syntax noprefix
.globl sysfunc
sysfunc:
push rbx
mov r8, rdx

mov rax, [rdi]
mov rbx, [rsi]
mov r9,  [rdx]
mov rdx, [rcx]

push rdi
push rsi
push r8
push rcx
push r11
syscall
pop r11
pop rcx
pop r8
pop rsi
pop rdi

mov [rdi], rax
mov [rsi], rbx
mov [r8], r9
mov [rcx], rdx

pop rbx
ret
