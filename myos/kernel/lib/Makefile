SRC_TOP = ../..
include $(SRC_TOP)/dec.mk

LIBNAME = lib

C_FILES      = $(wildcard *.c)
ASMOBJ_FILES = $(wildcard *.asm)
INCLUDE_DIR  = ../include

DELETE_TARGET = $(wildcard *.o) $(wildcard *.obj) $(wildcard *.a)



default: $(C_FILES:.c=.o) $(ASMOBJ_FILES:.asm=.obj)
	$(AR) $(ARFLAGS) $(LIBNAME).a *.o *.obj

clean:
	$(DELETE) $(DELETE_TARGET)

