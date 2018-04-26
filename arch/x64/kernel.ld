OUTPUT_FORMAT("elf64-x86-64");
ENTRY(x64_startup);

MEMORY {
  startup(rx): ORIGIN = 0x00010000, LENGTH = 1K
  kernel(rw): ORIGIN = 0xffff800000011000, LENGTH = 30K
}

SECTIONS {
  .startup : {
    arch/x64/startup.o(.text);
  } > startup

  .text : AT(0x00011000) {
    *(.text);
  } > kernel

  . = ALIGN(0x1000);

  .data :{
    *(.data);
  } > kernel

  .rodata :{
    *(.rodata*);
  } > kernel

  __bss_start = .;
  .bss :{
    *(.bss);
  } > kernel
  __bss_end = .;

  /DISCARD/ :{
    *(*.eh_frame);
  }
}
