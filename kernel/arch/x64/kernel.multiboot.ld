OUTPUT_FORMAT("elf64-x86-64");
ENTRY(x64_startup);

MEMORY {
    startup(rx): ORIGIN = 0x00100000, LENGTH = 1K
    /* FIXME: isolate .data and make this readonly */
    rom(rwx): ORIGIN = 0xffff800000110000, LENGTH = 8M
    ram(rw): ORIGIN = 0xffff800000080000, LENGTH = 256K
}

SECTIONS {
    .startup : {
        build/kernel/arch/x64/startup.o(.text);
    } > startup

    .text : AT(0x00110000) {
        *(.text);
        *(.data);
        *(.rodata*);
    } > rom

    .bss :{
        __bss = .;
        *(.bss);
        __bss_end = .;

        /* The kernel stack used in boot sequence and idle thread. */
        __boot_stack = .;
        . = ALIGN(8);
        . += 0x4000;
        __boot_stack_end = .;
    } > ram

    /DISCARD/ :{
        *(*.eh_frame);
    }
}
