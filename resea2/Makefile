# The default build target. This should come first not to accidentally define
# the default in local.mk.
.PHONY: default
default: build

# Build configuration default values. You can set them through the command line
# (e.g. "make V=1") or your own "local.mk".

# Verbose. Set "y" to enable verbose build output.
export V       ?=
# QEMU on GUI. Set "y" to launch QEMU as a GUI application.
export GUI     ?=
# Enable KVM. Set "y" to enable KVM acceleration in QEMU.
export KVM     ?=
# Enable GDB. Set "y" to enable GDB debugging in QEMU.
export GDB     ?=
# Release Build. Set "y" to build OS in release mode.
export RELEASE ?=
# Target CPU arch. Only "riscv64" is supported (for now).
export ARCH    ?= riscv64
# Target machine. Only "qemu" is supported (for now).
export MACHINE ?= qemu
# The release verison string.
export VERSION ?= v0.0.0

# Your own build options. Create the file by yourself if needed.
top := $(PWD)
-include $(top)/local.mk

# Disable builtin implicit rules and variables.
MAKEFLAGS += --no-builtin-rules --no-builtin-variables
.SUFFIXES:

# Suppress verbose output if $(V) is not set.
ifeq ($(V),)
.SILENT:
endif

machine_dir := arch/$(ARCH)/machines/$(MACHINE)

PROGRESS ?= printf "  \\033[96m%8s\\033[0m  %s\\033[0m\\n"
CARGO    ?= cargo +nightly
RUST_GDB ?= rust-gdb

CARGOFLAGS += -Z build-std=core,compiler_builtins,alloc
CARGOFLAGS += -Z build-std-features=compiler-builtins-mem
CARGOFLAGS += --target $(machine_dir)/$(ARCH).json
CARGOFLAGS += $(if $(RELEASE),--release,)

export CARGO_BUILD_DEP_INFO_BASEDIR=$(top)

tools := $(foreach dir,$(wildcard tools/*),$(notdir $(dir)))
tool_path = target/release/$(strip $(1))

#
#  Build Commands
#
.PHONY: build
build: kernel.elf .vscode/settings.json

.PHONY: run
run: build $(call tool_path, run_emulator)
	$(PROGRESS) "RUN" kernel.elf
	./$(call tool_path, run_emulator)         \
		--arch $(ARCH)                    \
		--machine $(MACHINE)              \
		$(if $(GUI),--gui,)               \
		$(if $(KVM),--kvm,)               \
		$(if $(GDB),--gdb,)               \
		$(if $(QEMU),--qemu $(QEMU),)     \
		kernel.elf

.PHONY: gdb
gdb:
	$(PROGRESS) "GDB" $(machine_dir)/gdbinit
	$(RUST_GDB) -q -x $(machine_dir)/gdbinit

.PHONY: coverage
coverage:
	RUSTFLAGS="-Zinstrument-coverage" $(CARGO) build
	LLVM_PROFILE_FILE="unittest-%p-%m.profraw" $(CARGO) test
	grcov . -s . --binary-path ./target/debug -t html --branch --ignore-not-existing -o ./target/debug/coverage
	echo "***"
	echo "*** Generated a coverage report in: target/debug/coverage/index.html"
	echo "***"

.PHONY: clean
clean:
	$(CARGO) clean
	rm -rf *.elf *.map

.PHONY: test
test:
	$(CARGO) test --features hosting

.PHONY: lint
lint:
	$(CARGO) clippy --features hosting

.PHONY: lint-and-fix
lint-and-fix:
	$(CARGO) clippy --fix --features hosting

target_dir = target/$(ARCH)/$(if $(RELEASE),release,debug)
kernel.elf: $(target_dir)/kernel
	cp $(target_dir)/kernel kernel.elf

$(target_dir)/kernel: Makefile
	$(PROGRESS) "CARGO" "kernel"
	$(CARGO) build $(CARGOFLAGS) \
		--features $(ARCH),$(ARCH)_$(MACHINE) \
		--manifest-path kernel/Cargo.toml

$(addprefix target/release/,$(tools)): Makefile
	$(PROGRESS) "HOSTBUILD" "$(@F)"
	cd tools/$(@F) && $(CARGO) build --release

.vscode/settings.json:
	$(PROGRESS) "GEN" $@
	cp .vscode/settings.recommended.json .vscode/settings.json

-include $(target_dir)/kernel.d
-include target/release/*.d
