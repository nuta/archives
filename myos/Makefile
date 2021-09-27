#
#
#  myos (/Makefile)
#
#

SRC_TOP = .
include $(SRC_TOP)/dec.mk

DELETE_TARGET = myos.img *.bin


default:
	$(MAKE) -C boot
	$(MAKE) -C kernel
	$(MAKE) $(DISKIMG_FILE)


run:
	$(MAKE)
	$(ECHO)
	$(ECHO) "starting the emulator..."
	$(EMU) $(EMU_OPTIONS)


clean:
	$(DELETE) $(DELETE_TARGET)
	$(MAKE)   -C boot   clean
	$(MAKE)   -C kernel clean


$(DISKIMG_FILE): $(BOOT_BIN) #$(KERNEL_BIN)
	$(ECHO) "  $(DISKIMG_FILE)"
	$(ATTACH)     $(SYSIMG_FILE) $(BOOT_BIN) $(KERNEL_BIN)
	$(MKRESFS)    $(FSIMG_FILE)   $(FILES)
	$(ATTACH)     $(DISKIMG_FILE) $(SYSIMG_FILE) $(FSIMG_FILE)
	$(ADDPADDING) $(DISKIMG_FILE) $(DISKIMG_FILESIZE)

