ENTRY(riscv64_boot)

KERNEL_BASE = 0xffffff8000000000;

SECTIONS {
    . = 0x80200000 + KERNEL_BASE;
    __kernel_memory = .;

    .text : AT(ADDR(.text) - KERNEL_BASE) {
        KEEP(*(.text.boot));
        *(.text .text.*);
    }

    . = ALIGN(16);
    .rodata : AT(ADDR(.rodata) - KERNEL_BASE) {
        *(.rodata .rodata.*);
    }

    . = ALIGN(16);
    .data : AT(ADDR(.data) - KERNEL_BASE) {
        *(.data .data.*);
    }

    . = ALIGN(16);
    .bss : AT(ADDR(.bss) - KERNEL_BASE) {
        __bss = .;
        *(.bss .bss.* .sbss .sbss.*);
        __bss_end = .;
    }

    __kernel_memory_end = .;

    /DISCARD/ : {
        *(.comment);
        *(.note.*);
        *(.debug.*);
        *(.debug_frame);
    }
}
