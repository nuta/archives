OUTPUT_FORMAT("elf32-littlearm");
ENTRY(arm_init);
 
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
    }
}

