all: bootx64.efi

.buildenv: tools/buildenv
	./tools/buildenv $@

VERSION = $(shell git rev-parse HEAD)

CC       := $(BUILD_PREFIX)$(CC)
CXX      := $(BUILD_PREFIX)$(CXX)
LD       := $(BUILD_PREFIX)$(LD)
OBJCOPY  ?= $(BUILD_PREFIX)objcopy
PYTHON   ?= python3

-include .buildenv
include mk/efijs.mk
include mk/gnuefi.mk
include mk/v8.mk
include mk/musl.mk
include mk/libcxx.mk
include mk/libcxxrt.mk

GCCFLAGS = -O2 -g -mno-red-zone -m64 -nostdinc \
           -fpic -fshort-wchar -fno-strict-aliasing -fno-merge-all-constants \
           -ffreestanding -fno-stack-protector -fno-stack-check \
           -MT $@ -MMD -MP -MF $(dir $@)/.$(notdir $@).deps \
           -D__EFIJS__ -D'__EFIJS_VERSION__="$(VERSION)"' -D_XOPEN_SOURCE=700 \
           -D_LIBCPP_NO_EXCEPTIONS -D_LIBCPP_HAS_NO_THREADS -D_LIBCPP_HAS_MUSL_LIBC \
           -D_LIBCPP_BUILD_STATIC -D_LIBCPP_NO_RTTI -D_LIBCPP_STD_VER=14 \
           -DCONFIG_X86_64 \
           -DENABLE_DEBUGGER_SUPPORT -DENABLE_DISASSEMBLER \
           -DV8_HOST_ARCH_X64 -DV8_TARGET_ARCH_X64 -DV8_OS_EFIJS=1 \
           -U__linux__ -U__STRICT_ANSI__

QEMUFLAGS = -bios tmp/OVMF.fd -drive file=fat:rw:boot -nographic

override CFLAGS += \
    -std=c11 \
    $(GCCFLAGS) $(EXTRA_GCCFLAGS) \
    -Iinclude \
    -Ivendor \
    -Ivendor/musl/src/internal \
    -Ivendor/musl/include \
    -Ivendor/musl/arch/x86_64 \
    -Ivendor/musl/arch/x86_64/bits \
    -Ivendor/musl/arch/generic \
    -Ivendor/gnuefi/lib \
    -Ivendor/gnuefi/inc \
    -Ivendor/gnuefi/inc/x86_64 \
    -Ivendor/gnuefi/inc/protocol

override CXXFLAGS += \
    -std=c++11 -nostdinc++ -fno-exceptions \
    $(GCCFLAGS) $(EXTRA_GCCFLAGS) \
    -Iinclude \
    -Ivendor \
    -Ivendor/libcxxrt/include \
    -Ivendor/libcxx/include \
    -Ivendor/musl/src/internal \
    -Ivendor/musl/include \
    -Ivendor/musl/arch/x86_64 \
    -Ivendor/musl/arch/x86_64/bits \
    -Ivendor/musl/arch/generic \
    -Ivendor/gnuefi/lib \
    -Ivendor/gnuefi/inc \
    -Ivendor/gnuefi/inc/x86_64 \
    -Ivendor/gnuefi/inc/protocol \
    -Ivendor/v8 \
    -Ivendor/v8/src \
    -Ivendor/v8/include \
    -Ivendor/trace_event

override LDFLAGS  += -Tvendor/gnuefi/gnuefi/elf_x86_64_efi.lds \
                     -nostdlib -shared -Bsymbolic --no-undefined --no-as-needed \
                     -Map=bootx64.map

$(V).SILENT:
.SUFFIXES:
.SECONDARY:
.POHNY: clean run test

clean:
	-rm -f bootx64.efi bootx64.so bootx64.map $(objs)

run: tmp/OVMF.fd boot/efi/boot/bootx64.efi
	$(CMDECHO) RUN boot/efi/boot/bootx64.efi
	$(PYTHON) ./tools/run.py "qemu-system-x86_64 $(QEMUFLAGS)"

test: tmp/OVMF.fd boot/efi/boot/bootx64.efi
	$(CMDECHO) RUN boot/efi/boot/bootx64.efi
	(sleep 15; echo -e "\x01cq") | qemu-system-x86_64 $(QEMUFLAGS)

boot/efi/boot/bootx64.efi: bootx64.efi
	mkdir -p $(dir $@)
	$(CMDECHO) CP $@
	cp $< $@

tmp/OVMF.fd:
	cd tmp && wget -U "" https://downloads.sourceforge.net/project/edk2/OVMF/OVMF-X64-r15214.zip
	cd tmp && unzip OVMF-X64-r15214.zip OVMF.fd

bootx64.so: $(objs)
	$(CMDECHO) LD $@
	$(LD) $(LDFLAGS) -o $@ $^

bootx64.efi: bootx64.so
	$(CMDECHO) OBJCOPY $@
	$(OBJCOPY)  -j .text -j .sdata -j .data -j .dynamic -j .dynsym \
                    -j .rel -j .rela -j .reloc \
                    --target=efi-app-x86_64 \
                    $< $@

$(objs): $(autogen_files)

%.o: %.c
	$(CMDECHO) CC $@
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.c
	$(CMDECHO) CC $@
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.S
	$(CMDECHO) CC $@
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.cc
	$(CMDECHO) CXX $@
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.cpp
	$(CMDECHO) CXX $@
	$(CXX) $(CXXFLAGS) -c $< -o $@

-include $(shell find . -name ".*.deps")