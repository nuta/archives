SRC_TOP = ../..
include $(SRC_TOP)/dec.mk

LIBNAME = net

C_FILES      = $(wildcard *.c) $(wildcard sys/*.c)
ASMOBJ_FILES = $(wildcard sys/*.asm)
INCLUDE_DIR  = ../include

DELETE_TARGET = $(wildcard *.o) $(wildcard sys/*.o) $(wildcard sys/*.obj)  $(wildcard *.a)


default: $(C_FILES:.c=.o) $(ASMOBJ_FILES:.asm=.obj)
	$(AR) $(ARFLAGS) $(LIBNAME).a *.o sys/*.o sys/*.obj

clean:
	$(DELETE) $(DELETE_TARGET)

