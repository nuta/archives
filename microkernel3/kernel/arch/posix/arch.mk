arch_include_dirs := .
arch_objs := main.o arch.o thread.o

override CFLAGS += -O0 -g3 -DARCH_POSIX
override LDFLAGS +=

ifeq ($(KERNEL_TEST), 1)
override CFLAGS += -fprofile-instr-generate -fcoverage-mapping
override LDFLAGS += -fprofile-instr-generate -fcoverage-mapping
endif

.PHONY: run
run:
	$(MAKE) build
	./tools/run-emulator.py ./build/kernel/kernel.elf

.PHONY: test
test:
	KERNEL_TEST=1 $(MAKE) build
	./tools/run-emulator.py --test ./build/kernel/kernel.elf

.PHONY: coverage
coverage:
	KERNEL_TEST=1 $(MAKE) build
	./tools/run-emulator.py --test ./build/kernel/kernel.elf
	$(LLVM_DIR)/llvm-profdata merge -sparse default.profraw -o default.profdata
	$(LLVM_DIR)/llvm-cov report ./build/kernel/kernel.elf \
		-instr-profile=default.profdata
	$(LLVM_DIR)/llvm-cov show -format=html ./build/kernel/kernel.elf \
		-output-dir=coverage \
		-instr-profile=default.profdata

.PHONY: lldb
lldb:
	KERNEL_TEST=1 $(MAKE) build
	lldb ./build/kernel/kernel.elf
