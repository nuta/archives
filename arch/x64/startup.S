.intel_syntax noprefix
.code16

#define MEMMAP_ADDR 0x8000

.globl x64_startup
x64_startup:
    cld
    mov esp, 0x7c00

    /* Get memory map. */
    mov ax, 0
    mov es, ax
    mov eax, 0xe820
    mov ebx, 0
    mov ecx, 64
    mov edx, 0x534d4150
    mov di, MEMMAP_ADDR
    int 0x15

    /* Open A20 Line. */
    in al, 0x92
    or al, 2
    out 0x92, al

    /* Prepare for RETF. */
    mov ax, 24
    lea edx, [protected_mode]
    push ax
    push edx

    /* Copy GDTR and GDT to a lower address. */
    mov ecx, 0x100
    mov edi, 0x7e00
    lea esi, [gdtr]
    and si, 0xffff
    mov ax, 0x1000
    mov ds, ax
    rep movsb

    /* Enable protected mode. */
    mov eax, cr0
    or eax, 1
    and  eax, 0x7fffffff

    mov dx, 0
    mov ds, dx
    lgdt [0x7e00]

    mov cr0, eax
    retf

.code32
protected_mode:
    mov ax, 16
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

construct_page_table:
    /* PML4: 0x00000000_00000000 */
    mov edi, 0xa000
    mov dword ptr [edi], 0xb003 /* Present, writable. */
    mov dword ptr [edi + 4], 0

    /* PML4: 0xffff8000_00000000 */
    mov edi, 0xa000 + 256 * 8
    mov dword ptr [edi], 0xb003 /* Present, writable. */
    mov dword ptr [edi + 4], 0

    /* PDPT */
    mov edi, 0xb000
    mov dword ptr [edi], 0xc003 /* Present, writable. */
    mov dword ptr [edi + 4], 0

    /* Page Directory */
    mov edi, 0xc000
    mov eax, 0x0083 /* Present, writable, page size is 2MB. */
    mov ecx, 100

write_entry:
    mov dword ptr [edi], eax
    mov dword ptr [edi + 4], 0
    add eax, 0x200000 /* 2MB */
    add edi, 8
    loop write_entry

enable_long_mode:
    /* Enable PAE and PGE. */
    mov eax, cr4
    or eax, 0xa0
    mov cr4, eax

    /* Set the page table address. */
    mov eax, 0xa000
    mov cr3, eax

    /* Enable long mode. */
    mov ecx, 0xc0000080
    rdmsr
    or eax, 0x0100
    wrmsr

    /* Prepare for RETF. */
    mov ax, 8
    lea edx, [long_mode]
    push ax
    push edx

    /* Enable paging. */
    mov eax, cr0
    or  eax, 0x80000000
    mov cr0, eax

    retf

.code64
long_mode:
    mov ax, 0
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov rax, x64_init_addr
    call rax

halt:
    hlt
    jmp halt


.align 8

x64_init_addr:
    .quad x64_init

gdtr:
    .word gdt_end - gdt - 1
    .quad 0x7e0a

gdt:
    .quad 0x0000000000000000 /* 0:  null descriptor */
    .quad 0x00af9a000000ffff /* 8:  64-bit code segment (kernel): 0x0000000000000000 - 0xffffffffffffffff */
    .quad 0x00cf92000000ffff /* 16: 64-bit data segment (kernel): 0x0000000000000000 - 0xffffffffffffffff */
    .quad 0x00cf9a000000ffff /* 24: 32-bit code segment (kernel): 0x00000000         - 0xffffffff */
gdt_end:
