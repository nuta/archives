ARCH ?= x64
COMMON_MK = $(shell pwd)/mk/common.mk

.PHONY: default build clean run test setup
default: build

ARCH_DIR = arch/$(ARCH)
CFLAGS := $(CFLAGS)
LDFLAGS := $(CFLAGS)

all_objs :=
all_libs :=
all_include_dirs :=

include kernel/build.mk arch/$(ARCH)/build.mk
include $(foreach lib, $(all_libs), libs/$(lib)/build.mk)

# Set `y' to suppress annoying build messages.
V =
$(V).SILENT:
.SECONDARY:

OBJCOPY ?= gobjcopy
PROGRESS ?= printf "  \033[1;35m%7s  \033[1;m%s\033[m\n"

build: kernel/kernel.elf

clean:
	rm -f */*.o */*/*.o */*/*/*.o \
		*/*.elf */*/*.elf */*/*/*.elf \
		*/*.img */*/*.img */*/*/*.img \
		*/*.tmp */*/*.tmp */*/*/*.tmp \
		*/*.bin */*/*.bin */*/*/*.bin

setup:
	brew install llvm binutils qemu mtools

kernel/kernel.elf: $(all_objs) $(ARCH_DIR)/kernel.ld
	$(PROGRESS) LD $@
	$(LD) $(LDFLAGS) --script $(ARCH_DIR)/kernel.ld -o $@ $(all_objs)

%.o: %.S Makefile
	$(PROGRESS) CC $@
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.c Makefile
	$(PROGRESS) CC $@
	$(CC) $(CFLAGS) $(addprefix -I,$(all_include_dirs)) -c -o $@ $<
