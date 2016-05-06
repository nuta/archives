OUTPUT_FORMAT("elf64-x86-64");
ENTRY(x86_start);

SECTIONS {

  . = 0x00100000;

  .bootstrap :{
    build/release/x86/src/init.o(.text);
    build/release/x86/src/init2.o(.text);
 }

 . += 0xffff800000000000;

 .text :{
   *(.text);
 }

 .data :{
   *(.data);
 }

 .rodata :{
   *(.rodata*);
 }

 .bss :{
   *(.bss);
 }

 /DISCARD/ :{
   *(*.eh_frame);
 }
}

