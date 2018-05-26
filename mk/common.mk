ARCH ?= x64
SERVERS ?=
BUILD_DIR ?= build

override LDFLAGS := $(LDFLAGS)
override CFLAGS := $(CFLAGS) \
	-std=c11 \
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
CC = /usr/local/opt/llvm/bin/clang
LD = /usr/local/opt/llvm/bin/ld.lld
DD = gdd
TAR = gtar
TOOLCHAIN_PREFIX = g
endif

PROGRESS ?= printf "  \033[1;35m%7s  \033[1;m%s\033[m\n"

tools/idl/parser/idlParser.py: tools/idl/idl.g4
	mkdir -p tools/idl/parser
	$(PROGRESS) ANTLR4 $@
	cd tools/idl && \
		$(ANTLR4) -Dlanguage=Python3 -o parser $(notdir $<)
	touch $(dir $@)__init__.py
