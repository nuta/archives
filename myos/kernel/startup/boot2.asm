[bits 32]

mov ax, 16
mov ds, ax
mov es, ax


;
;  ページングテーブルを作成
;

;ページマップ・レベル4・テーブル
mov  edi,   0x10000
mov  eax,   0x2000f   ;Present, Writable, User
mov  [edi], eax



;ページディレクトリ・ポインタ・テーブル
mov  edi,   0x20000
mov  eax,   0x3000f   ;Present, Writable, User
mov  [edi], eax



;ページディレクトリ
mov  edi,    0x30000
mov  eax,    0x00008f      ;Present, Writable, User, Page size:2MB
mov  [edi],  eax

mov  edi,    0x30000+8
mov  eax,    0x20008f      ;Present, Writable, User, Page size:2MB
mov  [edi],  eax


;PAE,PGE有効化
mov  eax, cr4
or   eax, 0xa0
mov  cr4, eax


;ページング有効化
mov  eax, 0x10000
mov  cr3, eax


;ロングモードに移行
mov   ecx, 0xc0000080
rdmsr
or    eax, 0x0100
wrmsr

push dword 0x08           ; CS
push dword boot3+0x8400   ; EIP

mov     eax, cr0
or      eax, 0x80000000
mov     cr0, eax
retf

times 0x400-($-$$) db 0
boot3:

