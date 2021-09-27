%include "kernel.inc"

[bits 64]



;
;  Function: asm_intNE2000Handler
;
;    ネットワークカードハンドラ
;
;  Parameters:
;
;    なし
;
;  Returns:
;
;    なし
;

global asm_intNE2000Handler
extern intNE2000Handler
asm_intNE2000Handler:
cli

push rax
push rbx
push rcx
push rdx
push rdi
push rsi
push r8

call intNE2000Handler

mov  al,   0x63
out  0xa0, al
mov  al,   0x62
out  0x20, al

;
;DPLが0に固定されているので、ユーザモード時は直す
;


mov rax, [rsp+88]

cmp rax, 0x20
jne .skip

mov rax, 0x23
mov [rsp+88], rax

.skip:

pop  r8
pop  rsi
pop  rdi
pop  rdx
pop  rcx
pop  rbx
pop  rax
iretq


