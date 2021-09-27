SRC_TOP = ..
include $(SRC_TOP)/dec.mk

DELETE_TARGET = $(wildcard *.bin)

default:
	$(AS) $(ASMBIN_FLAGS) ipl.asm -o $(BOOT_BIN)


clean:
	$(DELETE) $(DELETE_TARGET)

