[bits    16]
[org     0x7c00]



entrypoint:

cli

;レジスタ初期化
mov     ax, cs
mov     ss, ax
mov     ds, ax
mov     sp, 0x7c00
mov     ax, 0xb800
mov     gs, ax
mov     di, 0

mov     ah, 0x00
mov     al, 0x03
int     0x10

;カーネル読み込み
call load

;boot2へ移行
jmp dword  0:0x08000  



load:
jmp read13e

load_ret:
ret


;拡張INT 0x13を使った読み込み (ハードディスクおよびUSBメモリ用)
read13e:
mov  si, msgUsingINT13e
call print

;拡張INT 0x13が使えるか確認

mov ah, 0x41
mov bx, 0x55aa
int 0x13
jc  error
cmp bx, 0xaa55
jne error
and cl, 1
cmp cl, 1
jne error

mov ah, 0x42
mov al, 0
mov si, DAP
int 0x13
jc  error

jmp load_ret



error:
mov  si, msgLoadingError
call print

.hlt:
hlt
jmp  .hlt

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



DAP:
db	0x10		; パケットサイズ(=16)
db	0x00		; 予約
db	0xd0	    ; Blocks
db	0			; 予約
dw	0x8000		; 読み込みオフセット位置
dw	0x0000		; 読み込みセグメント位置
dd	1			; 読み込み先頭セクタ番号
dd	0			; 読み込み先頭セクタ番号上位32ビット



msgUsingINT13e      db "Using extended INT 0x13",    0x0d,0x0a,0
msgLoadingError     db 0x0d, 0x0a, "Loading error.", 0

times 510-($-$$) db 0
dw 0xaa55

