#
#
#  myos (/dec.mk)
#
#


TMP_DIR      = /tmp
BOOT_DIR     = $(SRC_TOP)/boot
KERNEL_DIR   = $(SRC_TOP)/kernel
TOOLS_DIR    = $(SRC_TOP)/tools
FILES_DIR    = $(SRC_TOP)/files

BOOT_BIN         = $(BOOT_DIR)/boot.bin
KERNEL_BIN       = $(KERNEL_DIR)/kernel.bin
STARTUP_BIN      = $(KERNEL_DIR)/startup/startup.bin
SYSIMG_FILE      = $(SRC_TOP)/sysimg.bin
FSIMG_FILE       = $(SRC_TOP)/fsimg.bin
DISKIMG_FILE     = $(SRC_TOP)/myos.img

DISKIMG_FILESIZE    = 2097152     # 2MB  (0x200000)
KERNEL_BIN_FILESIZE = 65536       # 64KB (0x10000)

FILES = $(wildcard $(FILES_DIR)/*)

AS          = nasm
CC          = gcc
LINK        = ld
AR          = ar
DELETE      = -rm
ECHO        = echo
EMU         = qemu-system-x86_64
ATTACH      = $(TOOLS_DIR)/attach.pl
ADDPADDING  = $(TOOLS_DIR)/addpadding.pl
ELF2BIN     = $(TOOLS_DIR)/elf2bin.pl
MKRESFS     = $(TOOLS_DIR)/mkmyfs.php


EMU_OPTIONS  = -hda $(DISKIMG_FILE) -k ja -m 128 -localtime \
               -d pcall,cpu_reset,int                       \
               -net nic,model=ne2k_pci -net user -net dump,file=net_dump -redir tcp:12345:10.0.2.15:80

ASMBIN_FLAGS = -f bin   -s -I $(INCLUDE_DIR)/

ASMOBJ_FLAGS = -f elf64 -s -I $(INCLUDE_DIR)/

CFLAGS       = -c -std=c99 -m64 -Os -pipe -I $(INCLUDE_DIR)     \
               -Wall -Wunused -Wcast-qual  -Wcast-align         \
               -Winit-self                                      \
               -Wunsafe-loop-optimizations                      \
               -Wmissing-include-dirs -Wunknown-pragmas         \
               -fno-common -fno-builtin -fno-builtin-function   \
               -fno-stack-protector -mno-red-zone               \
               -nostdlib -mcmodel=large -mno-red-zone -mno-mmx  \
               -mno-sse -mno-sse2 -mno-sse3 -mno-3dnow

LINKFLAGS    = -nostdlib -nodefaultlibs

ARFLAGS      = crs


%.bin: %.asm  $(wildcard $(INCLUDE_DIR)/*.inc)
	$(ECHO) "  $*.bin"
	$(AS) $(ASMBIN_FLAGS) $< -o $@

%.obj: %.asm  $(wildcard $(INCLUDE_DIR)/*.inc)
	$(ECHO) "  $*.obj"
	$(AS) $(ASMOBJ_FLAGS) $< -o $@

%.o: %.c      $(wildcard $(INCLUDE_DIR)/*.h)
	$(ECHO) "  $*.o"
	$(CC) $(CFLAGS) $< -o $@


