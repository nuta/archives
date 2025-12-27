OUTPUT_ARCH(aarch64)
ENTRY(_start)

KERNEL_BASE = 0xffffffe000000000;

SECTIONS {
    /* DTB is at 0x40000000 (QEMU virt machine) */

    . = 0x40200000 + KERNEL_BASE;
    __kernel_memory = .;

    .text : AT(ADDR(.text) - KERNEL_BASE) {
        *(.text.boot)
        *(.text .text.*)
    }

    . = ALIGN(16);
    .rodata : AT(ADDR(.rodata) - KERNEL_BASE) {
        *(.rodata .rodata.*)
    }

    . = ALIGN(16);
    .data : AT(ADDR(.data) - KERNEL_BASE) {
        *(.data .data.*)
    }

    . = ALIGN(16);
    .bss : AT(ADDR(.bss) - KERNEL_BASE) {
        *(.bss .bss.*)
    }

    __kernel_memory_end = .;

    /DISCARD/ : {
        *(.eh_frame)
    }
}
