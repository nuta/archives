OUTPUT_FORMAT(elf64-x86-64)
ENTRY(boot4)


SECTIONS{

  . = 0xc0000008c00;

  .text :{
	*(.entry);
	*(.text);
  }

  .data ALIGN   (0x10):{
	*(.data);
  }

  .rodata ALIGN (0x10):{
	*(.rodata);
  }

  . = 0xc0000300000;

  .bss ALIGN    (0x10):{
    *(.bss);
  }

  end = .;
}

