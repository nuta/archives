# Default values for build system.
export V ?=

# The default build target.
.PHONY: default
default: build

# Disable builtin implicit rules and variables.
MAKEFLAGS += --no-builtin-rules --no-builtin-variables
.SUFFIXES:

# Enable verbose output if $(V) is set.
ifeq ($(V),)
.SILENT:
endif

PROGRESS   := printf "  \\033[1;96m%8s\\033[0m  \\033[1;m%s\\033[0m\\n"

QEMU       ?= qemu-system-x86_64
QEMUFLAGS += -m 512 -serial mon:stdio
QEMUFLAGS += --bios /usr/share/qemu/OVMF.fd
QEMUFLAGS += -drive file=fat:rw:fat -no-reboot -boot d -d guest_errors,unimp
QEMUFLAGS += $(if $(GUI),,-nographic)

.PHONY: build
build:
	$(PROGRESS) "CARGO" build
	cargo +nightly build -Zbuild-std=core,alloc --target x86_64-unknown-uefi
	$(PROGRESS) "CP" fat/efi/boot/bootx64.efi
	mkdir -p fat/efi/boot
	cp target/x86_64-unknown-uefi/debug/kazari-on-uefi.efi fat/efi/boot/bootx64.efi

.PHONY: run
run: build
	$(PROGRESS) "QEMU"
	$(QEMU) $(QEMUFLAGS)
