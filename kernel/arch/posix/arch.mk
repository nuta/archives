arch_include_dirs := .
arch_objs := main.o arch.o

override CFLAGS += -O2 -g3 -DARCH_POSIX
override LDFLAGS +=

.PHONY: run
run:
	$(MAKE) build
	./build/kernel/kernel.elf
