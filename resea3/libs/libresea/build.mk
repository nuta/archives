objs := arch/$(ARCH)/start.o arch/$(ARCH)/syscall.o printf.o string.o list.o
include_dirs := include arch/$(ARCH)
subdirs := arch/$(ARCH)

include libs/lib.mk
