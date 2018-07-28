default: build

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
CC = /usr/local/opt/llvm/bin/clang
LD = /usr/local/opt/llvm/bin/ld.lld
DD = gdd
TAR = gtar
TOOLCHAIN_PREFIX = g
endif

ifeq ($(ARCH), posix)
CC = clang
LD = clang
endif

PROGRESS ?= printf "  \033[1;35m%7s  \033[1;m%s\033[m\n"
export ARCH BUIO:D+DOR CC LD PROGRESS TAR TOOLCHAIN_PREFIX OBJCOPY STRIP ANTLR4 DD

include kernel/kernel.mk

server_dirs := $(wildcard servers/*)
.PHONY: default build stubs clean run test $(server_dirs)

STUBS_DIR := $(BUILD_DIR)/stubs
stubs := $(wildcard interfaces/*.idl)
tools/idl/parser/idlParser.py: tools/idl/idl.g4
	mkdir -p tools/idl/parser
	$(PROGRESS) ANTLR4 tools/idl/parser
	cd tools/idl && \
		$(ANTLR4) -Dlanguage=Python3 -o parser $(notdir $<)
	touch $(dir $@)__init__.py

stubs: tools/idl/parser/idlParser.py tools/genstub.py $(stubs)
	$(PROGRESS) "GENSTUB" $(STUBS_DIR)/c
	./tools/genstub.py --out-dir $(STUBS_DIR)/c --lang c $(stubs)
	$(PROGRESS) "GENSTUB" $(STUBS_DIR)/rust
	./tools/genstub.py --out-dir libs/resea/src/interfaces --lang rust $(stubs)

build:
	@mkdir -p $(KFS_DIR)/servers
	@set -e; for server_dir in $(addprefix servers/, $(SERVERS)); do \
		$(MAKE) $$server_dir; \
	done
	$(MAKE) $(BUILD_DIR)/kernel/kernel.elf

$(server_dirs):
	$(PROGRESS) MAKE $@
	$(MAKE) -f servers/server.mk build DIR=$@ BUILD_DIR=$(BUILD_DIR) KFS_DIR=$(KFS_DIR)

clean:
	rm -rf $(BUILD_DIR)
