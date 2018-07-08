arch_include_dirs := .
arch_objs := startup.o init.o thread.o serial.o gdt.o idt.o tss.o paging.o \
	smp.o apic.o ioapic.o handler.o irq.o exception.o pic.o switch.o cpuvar.o \
	idle.o syscall.o usercopy.o vga.o putchar.o

disk_img = $(BUILD_DIR)/$(ARCH_DIR)/disk.img
QEMU ?= qemu-system-x86_64
BOCHS ?= bochs
override CFLAGS += -O2 -g3 --target=x86_64
override CFLAGS += -ffreestanding -fno-builtin -nostdinc -nostdlib -mcmodel=large
override CFLAGS += -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -mno-avx -mno-avx2
override LDFLAGS +=
QEMUFLAGS += -d cpu_reset -D qemu.log -nographic -cpu SandyBridge,rdtscp -rtc base=utc
QEMUFLAGS += -drive file=$(disk_img),if=virtio,format=raw -netdev user,id=net0 -device virtio-net-pci,netdev=net0

.PHONY: bochs
run:
	$(MAKE) build
	$(MAKE) $(disk_img)
	$(QEMU) $(QEMUFLAGS)

bochs: $(BUILD_DIR)/$(ARCH_DIR)/disk.img
	rm -f $(ARCH_DIR)/disk.img.lock
	$(BOCHS) -qf $(ARCH_DIR)/boot/bochsrc

test:
	$(MAKE) build
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
