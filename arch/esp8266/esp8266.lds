ENTRY(esp8266_boot)

MEMORY {
    dram  : org = 0x3fff9000, len = 0x3000
    iram0 : org = 0x40107000, len = 0x1000
    iram1 : org = 0x4026b000, len = 0xc000
}

PHDRS {
    iram0_phdr PT_LOAD;
    iram1_phdr PT_LOAD;
    dram_phdr  PT_LOAD;
}

SECTIONS {

    .iram0.text : {
        *lang*.o(.literal* .text*)
        *libgcc.a:_udivsi3.o(.literal* .text*)
        *libgcc.a:_umoddi3.o(.literal* .text*)
    } > iram0 : iram0_phdr

    .iram1.text : {
        *(.boot)
        *(EXCLUDE_FILE(*lang*.o *libgcc.a:_udivsi3.o *libgcc.a:_umoddi3.o) .literal* .text*)
    } > iram1 : iram1_phdr

    .data : ALIGN(4) {
        *(.data*)
        *(.rodata*)
    } > dram : dram_phdr

    .bss : ALIGN(4) {
        __bss_start = .;
        *(.bss*)
        __bss_end = .;

        __kmalloc_static_chunk = ALIGN(4);
        . = . + 0x2400;
        __kmalloc_static_chunk_end = .;

        __stack_bottom = .;
        . = . + 512;
        __stack = ALIGN(4);
    } > dram
}
