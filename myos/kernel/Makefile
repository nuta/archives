SRC_TOP = ..
include $(SRC_TOP)/dec.mk

INCLUDE_DIR  = include
C_KERNEL      = kernel_c.bin
DELETE_TARGET = $(wildcard *.o) $(wildcard *.bin) $(wildcard *.obj)



default:
	$(MAKE) -C core
	$(MAKE) -C lib
	$(MAKE) -C net
	$(MAKE) -C startup
	$(LINK) $(LINKFLAGS) -Map $(TMP_DIR)/myos.map -T $(INCLUDE_DIR)/kernel.ld startup/boot4.o core/*.o core/*.obj */*.a -o $(C_KERNEL)
	$(ELF2BIN) $(C_KERNEL)
	$(ATTACH) $(KERNEL_BIN) $(STARTUP_BIN) $(C_KERNEL)
	$(ADDPADDING) $(KERNEL_BIN) $(KERNEL_BIN_FILESIZE)

clean:
	$(MAKE) -C core      clean
	$(MAKE) -C lib       clean
	$(MAKE) -C net       clean
	$(MAKE) -C startup   clean
	$(DELETE) $(DELETE_TARGET)

