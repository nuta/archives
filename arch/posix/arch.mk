objs += \
    $(arch_dir)/panic.o     \
    $(arch_dir)/boot.o      \
     $(arch_dir)/mutex.o    \
    $(arch_dir)/printchar.o \
    $(arch_dir)/interrupt.o \
    $(arch_dir)/x86.o       \
    $(arch_dir)/thread.o

LD := ld

CFLAGS += -DARCH_POSIX -ggdb3 -Os

ifeq ($(shell uname -s), Linux)
CFLAGS += -pthread
endif

ifeq ($(shell uname -s), Darwin)
CC := gcc-6
endif

CPPFLAGS += -I$(arch_dir)

$(TARGET_FILE): $(objs)
	$(CMDECHO) CC $@
	$(CC) $(CFLAGS) -o $@ $(objs)

.PHONY: run
run:
	PYTHONPATH=$(makefile_dir) ./tools/run ./$(TARGET_FILE)
