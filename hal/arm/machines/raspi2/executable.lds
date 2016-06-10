OUTPUT_FORMAT("elf32-littlearm");
ENTRY(vectors);

SECTIONS
{
    . = 0x00008000;

    .text :{
        *(.boot);
        *(.text);
    }
    . = ALIGN(0x1000);

    .rodata :{
        *(.rodata)
    }


    . = ALIGN(0x1000);
    .data :{
        *(.data)
    }

    .bss :{
        *(.bss)

        /* stack */
        . = . + 0x4000;
        . = ALIGN(8);
        boot_stack = .;

        . = . + 0x4000;
        . = ALIGN(8);
        irq_stack = .;
    }
}

