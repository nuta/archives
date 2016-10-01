ENTRY(esp8266_boot)

MEMORY {
    dram : org = 0x3fff7000, len = 0x2000
    iram : org = 0x40105a00, len = 0x2600
}

PHDRS {
    iram_phdr PT_LOAD;
    dram_phdr PT_LOAD;
}

SECTIONS {

    .text : {
        *(.boot)
        *(.literal* .text*)
    } > iram : iram_phdr

    .data : ALIGN(4) {
        *(.data*)
        *(.rodata*)
    } > dram : dram_phdr

    .bss : ALIGN(4) {
        __bss_start = .;
        *(.bss*)
        __bss_end = .;

        __kmalloc_static_chunk = ALIGN(4);
        . = . + 0x1700; /* TODO: make this configurable */
        __kmalloc_static_chunk_end = .;

        __stack_bottom = .;
        . = . + 512;
        __stack = ALIGN(4);
    } > dram
}
