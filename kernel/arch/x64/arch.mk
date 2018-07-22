arch_include_dirs := .
arch_objs := startup.o init.o thread.o serial.o gdt.o idt.o tss.o paging.o \
	smp.o apic.o ioapic.o handler.o irq.o exception.o pic.o switch.o cpuvar.o \
	idle.o syscall.o usercopy.o vga.o putchar.o fpu.o pmc.o

disk_img = $(BUILD_DIR)/$(ARCH_DIR)/disk.img
QEMU ?= qemu-system-x86_64
BOCHS ?= bochs
override CFLAGS += -O2 -g3 --target=x86_64
override CFLAGS += -ffreestanding -fno-builtin -nostdinc -nostdlib -mcmodel=large
override CFLAGS += -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -mno-avx -mno-avx2
override LDFLAGS +=
KERNEL_LDFLAGS += --Map=$(BUILD_DIR)/kernel/kernel.map --script $(kernel_ld)
QEMUFLAGS += -d cpu_reset -D qemu.log -nographic -cpu SandyBridge,rdtscp -rtc base=utc
QEMUFLAGS += -drive file=$(disk_img),if=virtio,format=raw
QEMUFLAGS += -netdev user,id=net0,net=10.0.7.0/24,dhcpstart=10.0.7.15 -device virtio-net-pci,netdev=net0 -object filter-dump,id=qemu,netdev=net0,file=qemu.pcap

ifeq ($(MBRBOOT),)
kernel_ld = $(ARCH_DIR)/kernel.multiboot.ld
QEMUFLAGS += -kernel $(BUILD_DIR)/kernel/kernel.elf
else
kernel_ld = $(ARCH_DIR)/kernel.mbrboot.ld
endif

.PHONY: bochs
run:
	$(MAKE) build
	./$(ARCH_DIR)/tweak-elf-header.py $(BUILD_DIR)/kernel/kernel.elf
	$(MAKE) $(disk_img)
	$(QEMU) $(QEMUFLAGS)

bochs:
	$(MAKE) build
	$(MAKE) $(disk_img)
	rm -f $(ARCH_DIR)/disk.img.lock
	$(BOCHS) -qf $(ARCH_DIR)/boot/bochsrc

test:
	$(MAKE) build
	./$(ARCH_DIR)/tweak-elf-header.py $(BUILD_DIR)/kernel/kernel.elf
	$(MAKE) $(disk_img)
	(sleep 5; echo -e "\x01cq") | $(QEMU) $(QEMUFLAGS)

kernel-test:
	KERNEL_TEST=1 $(MAKE) build
	./$(ARCH_DIR)/tweak-elf-header.py $(BUILD_DIR)/kernel/kernel.elf
	$(MAKE) $(disk_img)
	(sleep 5; echo -e "\x01cq") | $(QEMU) $(QEMUFLAGS)

$(BUILD_DIR)/$(ARCH_DIR)/boot/mbr.elf: $(BUILD_DIR)/$(ARCH_DIR)/boot/mbr.o $(ARCH_DIR)/boot/mbr.ld
	$(PROGRESS) LD $@
	$(LD) $(LDFLAGS) --script $(ARCH_DIR)/boot/mbr.ld -o $@ $<

$(BUILD_DIR)/$(ARCH_DIR)/boot/mbr.bin: $(BUILD_DIR)/$(ARCH_DIR)/boot/mbr.elf
	$(PROGRESS) OBJCOPY $@
	$(OBJCOPY) -Obinary $< $@

$(disk_img): $(BUILD_DIR)/$(ARCH_DIR)/boot/mbr.bin $(BUILD_DIR)/kernel/kernel.elf
	$(PROGRESS) DD $@.tmp
	$(DD) if=/dev/zero of=$@.tmp bs=1M count=64
	$(PROGRESS) OFORMAT $@.tmp
	mformat -i $@.tmp -F -t 120000 -h 2 -s 4 ::
	$(PROGRESS) MCOPY $@.tmp
	mcopy -i $@.tmp $(BUILD_DIR)/kernel/kernel.elf ::/kernel.elf
	$(PROGRESS) CAT $@
	cat $(BUILD_DIR)/$(ARCH_DIR)/boot/mbr.bin $@.tmp > $@
