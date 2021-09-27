SRC_TOP = ../..
include $(SRC_TOP)/dec.mk

LIBNAME = startup

C_FILES      = $(wildcard *.c)
ASMBIN_FILES = boot1.asm boot2.asm boot3.asm
INCLUDE_DIR  = ../include

DELETE_TARGET = $(wildcard *.o) $(wildcard *.bin) $(wildcard *.a)



default: $(C_FILES:.c=.o) $(ASMBIN_FILES:.asm=.bin)
	$(AR) $(ARFLAGS) $(LIBNAME).a boot4.o
	$(ATTACH) $(STARTUP_BIN) boot1.bin boot2.bin boot3.bin

clean:
	$(DELETE) $(DELETE_TARGET)

