arch_include_dirs := .
arch_objs := main.o arch.o thread.o

override CFLAGS += -O0 -g3 -DARCH_POSIX
override LDFLAGS +=

.PHONY: run
run:
	$(MAKE) build
	./tools/run-emulator.py ./build/kernel/kernel.elf

.PHONY: test
test:
	KERNEL_TEST=1 $(MAKE) build
	./tools/run-emulator.py --test ./build/kernel/kernel.elf

.PHONY: lldb
lldb:
	KERNEL_TEST=1 $(MAKE) build
	lldb ./build/kernel/kernel.elf
