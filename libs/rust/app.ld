OUTPUT_FORMAT("elf64-x86-64");
ENTRY(start);

SECTIONS {

    . = 0x01000000;

    .text :{
        *(.text);
        *(.text.*);
    }

    .data ALIGN(0x1000) :{
        *(.data*);
        *(.data.*);
    }

    .rodata ALIGN(0x1000) :{
        *(.rodata*);
        *(.rodata.*);
    }

    .bss ALIGN(0x1000) :{
        *(.bss*);
    }

    /DISCARD/ :{
        *(*.eh_frame);
    }
}
