OUTPUT_FORMAT("elf64-x86-64");
ENTRY(start);

SECTIONS {
    . = 0x0000000001000000;

    .text : ALIGN(8) {
        __text = .;
        *(.text);
        *(.text.*);
        __text_end = .;
    }

    .rodata : ALIGN(8) {
        *(.rodata);
        *(.rodata.*);
    }

    . = ALIGN(4096);

    .data : ALIGN(8) {
        *(.data);
        *(.data.*);
    }

    .bss : ALIGN(8) {
        __bss = .;
        *(.bss);
        *(.bss.*);
        __bss_end = .;
    }
}
