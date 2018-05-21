ENTRY(main)

SECTIONS {
    . = 0x10000;
    .text :{
        *(.text*);
    }
}