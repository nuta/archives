efijs_objs = \
    src/main.o \
    src/boot.o \
    src/libc.o \
    src/libcxxrt.o \
    src/v8-platform.o \
    src/printf.o \
    src/strfmt.o \
    src/thread.o \
    src/mutex.o

$(efijs_objs): EXTRA_GCCFLAGS := -Wall -Wextra

objs += $(efijs_objs)
