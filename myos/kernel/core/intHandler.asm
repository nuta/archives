%include "kernel.inc"

[bits 64]


;
;  Function: asm_intTimerHandler
;
;    タイマハンドラ
;
;  Parameters:
;
;    なし
;
;  Returns:
;
;    なし
;

global asm_intTimerHandler
extern intTimerHandler
asm_intTimerHandler:
cli

push rax
push rbx
push rcx
push rdx
push rdi
push rsi
push r8

;
;タスクスイッチ用にレジスタを保存
;

push rdi
mov  rdi,  REGS_SAVE_ADDRESS
mov [rdi+16],   rax
mov [rdi+24],   rbx
mov [rdi+32],   rcx
mov [rdi+40],   rdx
mov [rdi+56],   rsi
mov [rdi+64],   rbp
mov [rdi+80],   r8
mov [rdi+88],   r9
mov [rdi+96],   r10
mov [rdi+104],  r11
mov [rdi+112],  r12
mov [rdi+120],  r13
mov [rdi+128],  r14
mov [rdi+136],  r15

;RDI
pop  rax
mov [rdi+48],   rax

;RSP
mov rax, [rsp+80]
mov [rdi+72],   rax


;RIP
mov rax,        [rsp+56]
mov [rdi+0],    rax

;CR3
mov rax,        cr3
mov [rdi+8],    rax


mov  al, 0x60
out  0x20, al

call intTimerHandler


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




;例外ハンドラを作るマクロ
%macro make_asm_exception 1
  global asm_exception%1
  extern exception%1

 asm_exception%1:
  cli

  mov rdi, [esp]
  call exception%1
  add rsp, 8

  iretq
%endmacro



;例外ハンドラ
make_asm_exception 0
make_asm_exception 1
make_asm_exception 2
make_asm_exception 3
make_asm_exception 4
make_asm_exception 5
make_asm_exception 6
make_asm_exception 7
make_asm_exception 8
make_asm_exception 9
make_asm_exception 10
make_asm_exception 11
make_asm_exception 12
make_asm_exception 13
make_asm_exception 14
make_asm_exception 15
make_asm_exception 16
make_asm_exception 17
make_asm_exception 18
make_asm_exception 19

