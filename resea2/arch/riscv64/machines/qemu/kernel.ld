OUTPUT_ARCH("riscv")
ENTRY(boot)

SECTIONS {
    . = 0x80000000;
    .boot : {
        KEEP(*(.boot .boot.*));
    }

    .text : ALIGN(4096) {
        *(.text .text.*);
    }

    .rodata : ALIGN(4096) {
      *(.rodata .rodata.*);
    }

    .data : ALIGN(4096) {
        *(.data .data.*);
    }

    .bss : ALIGN(4096) {
        *(.bss .bss.*);
    }

    . = ALIGN(4096);
    __kernel_end = .;
}
