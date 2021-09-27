%include "kernel.inc"

[bits 64]



;
;  Function: asm_cpuid
;
;    CPUID命令を実行する
;
;  Parameters:
;
;    rdi  - RAXレジスタのデータ
;    rsi  - EAXレジスタの保存先へのポインタ
;    rdx  - EBXレジスタの保存先へのポインタ
;    rcx  - ECXレジスタの保存先へのポインタ
;    r8   - EDXレジスタの保存先へのポインタ
;
;  Returns:
;
;    なし
;

global asm_cpuid
asm_cpuid:

push  r8
push  rcx
push  rdx
push  rsi
push  rdi

pop   rax
cpuid

pop   r9
mov   [r9], eax

pop   r9
mov   [r9], ebx

pop   r9
mov   [r9], ecx

pop   r9
mov   [r9], edx

ret



;
;  Function: asm_hlt
;
;    HLT命令を実行する
;
;  Parameters:
;
;    なし
;
;  Returns:
;
;    なし
;

global asm_hlt
asm_hlt:

hlt
ret




;
;  Function: asm_cli
;
;    CLI命令を実行する
;
;  Parameters:
;
;    なし
;
;  Returns:
;
;    なし
;

global asm_cli
asm_cli:

cli
ret




;
;  Function: asm_sti
;
;    STI命令を実行する
;
;  Parameters:
;
;    なし
;
;  Returns:
;
;    なし
;

global asm_sti
asm_sti:

sti
ret



;
;  Function: asm_stihlt
;
;    STI命令をHLT命令の直前に実行する
;
;  Parameters:
;
;    なし
;
;  Returns:
;
;    なし
;

global asm_stihlt
asm_stihlt:
sti
hlt
ret




;
;  Function: asm_out8
;
;    OUT命令を実行する
;
;  Parameters:
;
;    sil - データ
;    di  - ポート
;
;  Returns:
;
;    なし
;

global asm_out8
asm_out8:

mov  al, sil
mov  dx, di
out  dx, al
ret




;
;  Function: asm_in8
;
;    IN命令を実行する
;
;  Parameters:
;
;    di  - ポート
;
;  Returns:
;
;    読み取ったデータ
;

global asm_in8
asm_in8:

mov  rax,0
mov  dx, di
in   al, dx
ret




;
;  Function: asm_lgdt
;
;    LGDT命令を実行する
;
;  Parameters:
;
;    rdi  - LGTRの内容へのアドレス
;
;  Returns:
;
;    なし
;

global asm_lgdt
asm_lgdt:

lgdt [rdi]
ret




;
;  Function: asm_lidt
;
;    LIDT命令を実行する
;
;  Parameters:
;
;    rdi  - LDTRの内容へのアドレス
;
;  Returns:
;
;    なし
;

global asm_lidt
asm_lidt:

lidt [rdi]
ret




;
;  Function: asm_ltr
;
;    LTR命令を実行する
;
;  Parameters:
;
;    di  - TSSセレクタ
;
;  Returns:
;
;    なし
;

global asm_ltr
asm_ltr:

ltr di
ret




;
;  Function: asm_setCR3
;
;    CR3レジスタをセットする
;
;  Parameters:
;
;    rdi  - CR3レジスタのデータ
;
;  Returns:
;
;    なし
;

global asm_setCR3
asm_setCR3:

mov  rax, rdi
mov  cr3, rax
ret




;
;  Function: asm_loadCR2
;
;    CR2レジスタを読み込む
;
;  Parameters:
;
;    なし
;
;  Returns:
;
;    CR2レジスタのデータ
;

global asm_loadCR2
asm_loadCR2:

mov rax, cr2
ret

