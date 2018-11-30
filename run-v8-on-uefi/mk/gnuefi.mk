gnuefi_objs = \
    vendor/gnuefi/gnuefi/reloc_x86_64.o \
    vendor/gnuefi/lib/boxdraw.o \
    vendor/gnuefi/lib/cmdline.o \
    vendor/gnuefi/lib/console.o \
    vendor/gnuefi/lib/crc.o \
    vendor/gnuefi/lib/data.o \
    vendor/gnuefi/lib/debug.o \
    vendor/gnuefi/lib/dpath.o \
    vendor/gnuefi/lib/error.o \
    vendor/gnuefi/lib/event.o \
    vendor/gnuefi/lib/exit.o \
    vendor/gnuefi/lib/guid.o \
    vendor/gnuefi/lib/hand.o \
    vendor/gnuefi/lib/hw.o \
    vendor/gnuefi/lib/init.o \
    vendor/gnuefi/lib/lock.o \
    vendor/gnuefi/lib/misc.o \
    vendor/gnuefi/lib/print.o \
    vendor/gnuefi/lib/runtime/efirtlib.o \
    vendor/gnuefi/lib/runtime/rtdata.o \
    vendor/gnuefi/lib/runtime/rtlock.o \
    vendor/gnuefi/lib/runtime/rtstr.o \
    vendor/gnuefi/lib/runtime/vm.o \
    vendor/gnuefi/lib/smbios.o \
    vendor/gnuefi/lib/sread.o \
    vendor/gnuefi/lib/str.o \
    vendor/gnuefi/lib/x86_64/callwrap.o \
    vendor/gnuefi/lib/x86_64/initplat.o \
    vendor/gnuefi/lib/x86_64/math.o \
    vendor/gnuefi/lib/x86_64/efi_stub.o \
    vendor/gnuefi/gnuefi/crt0-efi-x86_64.o

objs += $(gnuefi_objs)