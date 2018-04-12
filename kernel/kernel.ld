ENTRY(boot)

SECTIONS {
    . = 0x10000;
    .text :{
        *(.text*);
    }
}