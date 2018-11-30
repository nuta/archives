ARCH ?= x64
SERVERS ?=
BUILD_DIR ?= build

override LDFLAGS := $(LDFLAGS)
override CFLAGS := $(CFLAGS) \
	-std=c17 \
    -Wall \
	-Werror=implicit-function-declaration \
	-Werror=int-conversion \
	-Werror=incompatible-pointer-types \
	-Werror=shift-count-overflow \
	-Werror=shadow

V =
$(V).SILENT:
.SECONDARY:
.SUFFIXES:

ANTLR4 ?= antlr4
OBJCOPY ?= $(TOOLCHAIN_PREFIX)objcopy
STRIP ?= $(TOOLCHAIN_PREFIX)strip
DD ?= dd
TAR ?= tar

CC = clang
LD = ld.lld

ifeq ($(shell uname), Darwin)
LLVM_DIR = /usr/local/opt/llvm/bin
CC = $(LLVM_DIR)/clang
LD = $(LLVM_DIR)/ld.lld
DD = gdd
TAR = gtar
TOOLCHAIN_PREFIX = g
endif

ifeq ($(ARCH), posix)
LD = $(CC)
endif

PROGRESS ?= printf "  \033[1;35m%7s  \033[1;m%s\033[m\n"
