.PHONY: default build clean run bochs test setup
default: build

# Set `y' to suppress annoying build messages.
V =

override CFLAGS += -O2 -Wall -Wextra -Werror -g3
override LDFLAGS +=

ifeq ($(shell uname), Darwin)
CC = /usr/local/opt/llvm/bin/clang --target=x86_64
LD = sh -c 'exec -a ld.lld /usr/local/opt/llvm/bin/lld $$*'
endif

OBJCOPY ?= gobjcopy
PROGRESS ?= printf "  \033[1;35m%7s  \033[1;m%s\033[m\n"
objs =

$(V).SILENT:
.SECONDARY:

build: disk.img

clean:
	rm -rf *.img *.o *.elf *.bin

bochs: disk.img
	rm -rf disk.img.lock
	bochs -qf bochsrc

run: disk.img
	qemu-system-x86_64 -hda disk.img -nographic

test: disk.img
	(sleep 3; echo -e "\x01cq") | qemu-system-x86_64 -hda disk.img -nographic

setup:
	brew install llvm binutils qemu mtools bochs

mbr.o: mbr.S
	$(PROGRESS) CC $@
	$(CC) $(CFLAGS) -c -o $@ $<

mbr.elf: mbr.o mbr.ld
	$(PROGRESS) LD $@
	$(LD) $(LDFLAGS) --script mbr.ld -o $@ $<

mbr.bin: mbr.elf
	$(PROGRESS) OBJCOPY $@
	$(OBJCOPY) -Obinary $< $@

kernel/kernel.elf: kernel/kernel.o kernel/kernel.ld
	$(PROGRESS) LD $@
	$(LD) $(LDFLAGS) --script kernel/kernel.ld -o $@ $<

disk.img: mbr.bin kernel/kernel.elf
	$(PROGRESS) DD $@.tmp
	dd if=/dev/zero of=$@.tmp bs=1m count=64
	$(PROGRESS) OFORMAT $@.tmp
	mformat -i $@.tmp -F -t 120000 -h 2 -s 4 ::
	$(PROGRESS) MCOPY $@.tmp
	mcopy -i $@.tmp kernel/kernel.elf ::/kernel.elf
	$(PROGRESS) CAT $@
	cat mbr.bin $@.tmp > $@

%.o: %.S Makeile
	$(PROGRESS) CC $@
	$(CC) $(CFLAGS) -c -o $@ $<