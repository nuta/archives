OUTPUT_FORMAT("elf64-x86-64");
ENTRY(x86_start);

SECTIONS{

  . = 0x00100000;

  .bootstrap :{
    *(.boot);
    *(.boot2);
  }

  . += 0xffff800000000000;

  .text : AT(ADDR(.text) - 0xffff800000000000) {
    *(.text);
  }

  .data : AT(ADDR(.data) - 0xffff800000000000) {
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

