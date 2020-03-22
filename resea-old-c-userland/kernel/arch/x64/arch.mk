# Specify "linux" toolchain to use the KASan.
KERNEL_CFLAGS += --target=x86_64-pc-linux-elf
KERNEL_CFLAGS += -mcmodel=large -mno-red-zone
KERNEL_CFLAGS += -mno-mmx -mno-sse -mno-sse2 -mno-avx -mno-avx2

ifeq ($(BUILD), debug)
KERNEL_CFLAGS += -fsanitize=undefined,kernel-address
KERNEL_CFLAGS += -mllvm -asan-instrumentation-with-call-threshold=0
KERNEL_CFLAGS += -mllvm -asan-globals=false
KERNEL_CFLAGS += -mllvm -asan-stack=false
KERNEL_CFLAGS += -mllvm -asan-use-after-return=false
KERNEL_CFLAGS += -mllvm -asan-use-after-scope=false
endif

HDD_DIR = $(BUILD_DIR)/hdd
BOCHS ?= bochs
QEMU ?= qemu-system-x86_64
QEMUFLAGS += -smp 2 -m 512 -cpu IvyBridge,rdtscp -rtc base=utc -serial mon:stdio
QEMUFLAGS += -no-reboot -device isa-debug-exit,iobase=0xf4,iosize=0x04
ifdef CONFIG_VIRTIO_BLK_SERVER
QEMUFLAGS += -drive id=hdd0,if=none,file=fat:rw:$(HDD_DIR)
QEMUFLAGS += -device virtio-blk-pci,drive=hdd0
endif
QEMUFLAGS += -boot d
QEMUFLAGS += $(if $(NOGUI),-nographic,)


objs += \
	kernel/arch/string.o \
	kernel/arch/x64/boot.o \
	kernel/arch/x64/apic.o \
	kernel/arch/x64/asan.o \
	kernel/arch/x64/handler.o \
	kernel/arch/x64/interrupt.o \
	kernel/arch/x64/paging.o \
	kernel/arch/x64/screen.o \
	kernel/arch/x64/serial.o \
	kernel/arch/x64/setup.o \
	kernel/arch/x64/mp.o \
	kernel/arch/x64/thread.o

.PHONY: bochs run iso
bochs: $(BUILD_DIR)/resea.iso
	$(PROGRESS) GEN $(BUILD_DIR)/kernel.symbols
	$(NM) $(BUILD_DIR)/kernel.elf | awk '{ print $$1, $$3 }' > $(BUILD_DIR)/kernel.symbols
	$(PROGRESS) BOCHS $<
	$(BOCHS) -qf misc/bochsrc

run: $(BUILD_DIR)/resea.iso
	$(PROGRESS) POPULATE $(HDD_DIR)
	mkdir -p $(HDD_DIR)
	echo "Hello from virtio-blk!" > $(HDD_DIR)/hello.txt
	$(PROGRESS) QEMU $(BUILD_DIR)/kernel.iso
	$(QEMU) $(QEMUFLAGS) -cdrom $<

iso: $(BUILD_DIR)/resea.iso

$(BUILD_DIR)/resea.iso: build
	mkdir -p $(BUILD_DIR)/isofiles/boot/grub
	cp misc/grub.cfg $(BUILD_DIR)/isofiles/boot/grub/grub.cfg
	cp $(BUILD_DIR)/kernel.elf $(BUILD_DIR)/isofiles/kernel.elf
	$(PROGRESS) GRUB $@
	$(GRUB_PREFIX)grub-mkrescue --xorriso=$(PWD)/misc/xorriso-wrapper -o $@ $(BUILD_DIR)/isofiles
