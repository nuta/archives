[bits 64]

;
;  Function: asm_switchTask
;
;    タスクスイッチをする
;
;  Parameters:
;
;    dsc    -  st_Registers構造体へのポインタ
;
;  Returns:
;
;    なし
;
;

global asm_switchTask
asm_switchTask:

mov  rax,  rdi
push qword [rax+64]
mov  rsi,  rax

mov  rax, [rsi+16]
mov  rbx, [rsi+24]
mov  rcx, [rsi+32]
mov  rdx, [rsi+40]
mov  rdi, [rsi+48]
mov  rbp, [rsi+64]
mov  r8,  [rsi+80]
mov  r9,  [rsi+88]
mov  r10, [rsi+96]
mov  r11, [rsi+104]
mov  r12, [rsi+112]
mov  r13, [rsi+120]
mov  r14, [rsi+128]
mov  r15, [rsi+136]


mov  [rsp+8], rax
mov  [rsp+16], rbx


mov  rax, [rsi]
mov  rbx, [rsi+72]

pop  rsi

push qword 0x0000000000000020+3 ;SS
push qword rbx                  ;RSP
push qword 0x0000000000000202   ;RFLAGS
push qword 0x0000000000000018+3 ;CS
push qword rax                  ;RIP


mov  rax, [rsp+0+40]
mov  rbx, [rsp+8+40]

iretq




;
;  Function: asm_APIEntry
;
;    APIのエントリーポイント
;
;  Parameters:
;
;    なし
;
;  Returns:
;
;    なし
;

global asm_APIEntry
extern APIEntry
asm_APIEntry:
cli

push rcx

mov rcx, r11
mov rdx, r10
mov rsi, r9
mov rdi, r8

call APIEntry

pop  rcx

sysret




;
;  Function: asm_initSYSCALL
;
;    SYSCALL命令の設定をする
;
;  Parameters:
;
;    rdi  - APIエントリーポイントのアドレス
;    rsi  - カーネルモード用のコードセグメントセレクタ
;    rdx  - ユーザモード用のコードセグメントセレクタ
;    rcx  - RFLAGSレジスタ
;
;  Returns:
;
;    なし
;

global asm_initSYSCALL
asm_initSYSCALL:

push rcx
push rdx
push rsi
push rdi

;
; RIPをセット
;

mov  ecx, 0xc0000082
pop  r8


;EAXが下位32ビット、EDXが上位32ビット
mov  eax, r8d

shr  r8,  32
mov  edx, r8d

wrmsr



;
; CSをセット
;

mov  ecx, 0xc0000081

pop r8
pop r9


;カーネルモード用CSをセット
mov  edx, r8d


;ユーザモード用CSをセット
shl r9d, 16
or  edx, r9d

wrmsr



;
; EFLAGSをセット
;

pop  r8

mov  ecx, 0xc0000084

mov  eax, r8d
shr  r8,  32
mov  edx, r8d

wrmsr



;
; SYSCALL命令を有効化
;

mov ecx, 0xc0000080
rdmsr

or  eax, 1
wrmsr

ret

