[bits 16]
[org  0]



;
;  レジスタ初期化
;

mov  ax, 0x800
mov  ds, ax
mov  ax, 0
mov  ss, ax



;
;  画面モードを設定
;

mov  ah, 0x00
mov  al, 0x03
int  0x10




;
;  A20有効化
;

call  waitKeyboard
mov   al,   0xd1
out   0x64, al
call  waitKeyboard
mov   al,   0xdf
out   0x60, al




;
;  CPUがロングモードに対応しているかチェック
;

checkCPUID:

;CPUID命令をサポートしているかチェック
.check1:

pushfd
pop  eax

or  eax, 0x200000

push  eax
popfd


pushfd
pop  eax

shr  eax, 21
and  eax, 1


jnz  .check2


mov  esi, msgCPUIDNotSupported
call print
jmp  halt


;EAXの初期値の最大値をチェック
.check2:

mov eax, 0x80000000
cpuid

cmp eax, 0x80000001
jae .check3


mov  esi, msgCPUIDNotSupported
call print
jmp  halt



;SYSCALL/SYSRET命令に対応しているかチェック
.check3:

mov eax, 0x80000001
cpuid

mov  eax, edx
shr  eax, 11
and  eax, 1
jnz  .check4


mov  esi, msgSYSCALLNotSupported
call print
jmp  halt



;ロングモードに対応しているかチェック
.check4:

mov  eax, edx
shr  eax, 29
and  eax, 1
jnz  checkCPUID_end


mov  esi, msgLongModeNotSupported
call print
jmp  halt


checkCPUID_end:





;
;  プロテクトモードへ移行
;

mov     eax, cr0
or      eax, 0x00000001  ;プロテクトモード
mov     cr0, eax


;GDTを読み込む
loadGDT:
lgdt    [GDTR]


;boot2に移行
jmp dword 24:0x8400




;------------------------------------





halt:
hlt
jmp halt



print:
push ax
push bx
mov  ah, 0x0e
mov  bh, 0
mov  bl, 15

.loop:
mov  al, [si]
cmp  al, 0
je   .ret
int  0x10
inc  si
jmp  .loop

.ret:
pop  bx
pop  ax
ret



waitKeyboard:
in      al, 0x64
and     al, 0x02
in      al, 0x60
jnz     waitKeyboard
ret

;------------------------------------

msgCPUIDNotSupported    db "CPUID Instruction not supported!"           ,0x0d,0x0a,0x00
msgSYSCALLNotSupported  db "SYSCALL/SYSRET Instruction not supported!"  ,0x0d,0x0a,0x00
msgLongModeNotSupported db "Long mode not supported!"                   ,0x0d,0x0a,0x00

;------------------------------------


align 8

GDTR:
dw   4*8 - 1
dq   GDT+0x8000

align 8


GDT:
dw 0x0000, 0x0000, 0x0000, 0x00
dw 0xffff, 0x0000, 0x9a00, 0xaf
dw 0xffff, 0x0000, 0x9200, 0x8f
dw 0xffff, 0x0000, 0x9a00, 0xcf


times 0x400-($-$$) db 0

