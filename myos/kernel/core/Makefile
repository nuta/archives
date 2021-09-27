SRC_TOP = ../..
include $(SRC_TOP)/dec.mk

DELETE_TARGET = $(wildcard *.o) $(wildcard *.obj) $(wildcard *.a)

INCLUDE_DIR  = ../include/

C_FILES      = $(wildcard *.c)
ASMOBJ_FILES = $(wildcard *.asm)


all: $(C_FILES:.c=.o) $(ASMOBJ_FILES:.asm=.obj)

clean:
	$(DELETE) $(DELETE_TARGET)

