OUTPUT_FORMAT("elf64-x86-64");
ENTRY(x86_start);

SECTIONS {

  . = 0x00100000;

  .bootstrap :{
    build/release/x86/src/start.o(.text);
    build/release/x86/src/_start.o(.text);
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

