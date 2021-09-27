[bits 64]


boot3:
mov ax, 0
mov ds, ax
mov es, ax
mov fs, ax
mov gs, ax
mov ss, ax

mov rsp, 0x7c00


;
;  ページングテーブルにカーネル用の空間(0xc00000000000〜)を追加
;

;ページマップ・レベル4・テーブル
mov  edi,   0x10000+0x40*3   ;???
mov  eax,   0x4000f          ;Present, Writable, User
mov  [edi], eax



;ページディレクトリ・ポインタ・テーブル
mov  edi,   0x40000
mov  eax,   0x5000f   ;Present, Writable, User
mov  [edi], eax



;ページディレクトリ
mov  edi,    0x50000
mov  eax,    0x018f       ;Present, Writable, User, Page size:2MB
mov  [edi],  eax

mov  edi,    0x50000+8
mov  eax,    0x20018f     ;Present, Writable, User, Page size:2MB
mov  [edi],  eax

mov  edi,    0x50000+16
mov  eax,    0x40018f     ;Present, Writable, User, Page size:2MB
mov  [edi],  eax

mov  edi,    0x50000+24
mov  eax,    0x60018f     ;Present, Writable, User, Page size:2MB
mov  [edi],  eax


mov rax, 0x10000
mov cr3, rax




;
; PICの初期化
;


mov  al, 0xff
out  0xa1, al
nop
out  0x21, al

mov  al, 0x11
out  0x20, al
nop
out  0xa0, al

mov  al, 0x20
out  0x21, al
mov  al, 0x28
out  0xa1, al

mov  al, 0x04
out  0x21, al
mov  al, 0x02
out  0xa1, al

mov  al, 0x01
out  0x21, al
nop
out  0xa1, al

mov  rsp, 0xc0000007c00

mov  rax, 0xc0000008c00
jmp  rax



times 0x400-($-$$) db 0

;
;  このバイナリファイルの直後にboot4がある
;

boot4:

