.intel_syntax noprefix

#define MEMMAP_ADDR 0x8000

/* Boot from mbr.S */
.globl x64_startup
x64_startup:
    /*
     *  There are two following conditions:
     *
     *    - Booted from mbr.S: We're in the real mode
     *    - Booted from GRUB: We're in the protected mode
     *
     *  The following instructions written by `.byte's determine
     *  the condition and go into the appropriate boot code.
     *
     */
     .byte 0x0f, 0x20, 0xc0 /* mov eax, cr0 */

    /*
     * In protected mode there instructions are decoded as: test eax, 0xd740001
     */
    .byte 0xa9, 0x01, 0x00 /* test, ax, 1 */
    .byte 0x74, multiboot_trampoline_end - multiboot_trampoline /* jz boot_from_mbrboot */

multiboot_trampoline:
.code32
    lea edx, [multiboot_entry]
    mov esp, 0x7c00
    push edx
    ret
multiboot_trampoline_end:

boot_from_mbrboot:
.code16
    cli
    cld
    mov sp, 0x7c00

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
.align 8
multiboot2:
    .long 0xe85250d6    /* magic */
    .long 0             /* arch (i386) */
    .long multiboot2_end - multiboot2
    .long   -(0xe85250d6 + 0 + (multiboot2_end - multiboot2))  /* checksum */

    /* terminator */
    .word 0
    .word 0
    .word 0
multiboot2_end:

/* Old fashion multiboot header for QEMU. */
.align 8
old_multiboot:
    .long 0x1badb002    /* magic */
    .long 0             /* flags */
    .long -(0x1badb002 + 0) /* checksum */
old_multiboot_end:

/* Boot from GRUB. We are already in the protected mode. */
multiboot_entry:
    cli
    cld
    mov esp, 0x7c00

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
    rep movsb

    lgdt [0x7e00]
    retf

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

    /* Clear .bss section */
    mov al, 0x00
    mov rdi, bss_addr
    mov rcx, bss_end_addr
    sub rcx, rdi
    rep stosb

    mov rsp, boot_stack_addr
    mov rax, x64_init_bsp_addr
    call rax

halt:
    hlt
    jmp halt

.align 8

bss_addr:
    .quad __bss

bss_end_addr:
    .quad __bss_end

x64_init_bsp_addr:
    .quad x64_init_bsp

boot_stack_addr:
    .quad __boot_stack_end

gdtr:
    .word gdt_end - gdt - 1
    .quad 0x7e0a

gdt:
    .quad 0x0000000000000000 /* 0:  null descriptor */
    .quad 0x00af9a000000ffff /* 8:  64-bit code segment (kernel): 0x0000000000000000 - 0xffffffffffffffff */
    .quad 0x00cf92000000ffff /* 16: 64-bit data segment (kernel): 0x0000000000000000 - 0xffffffffffffffff */
    .quad 0x00cf9a000000ffff /* 24: 32-bit code segment (kernel): 0x00000000         - 0xffffffff */
gdt_end:

