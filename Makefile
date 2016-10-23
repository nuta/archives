# TODO: remake all on config changes

VERBOSE     ?=
BUILD_DIR   ?= build
LANGS       ?= cpp
CMDECHO     ?= printf "  \033[1;35m%7s    \033[0;33m%s\033[m\n"
MKDIR       ?= mkdir
TARGET_FILE ?= $(BUILD_DIR)/$(target).elf

makefile_dir = $(PWD)
apps_dir = $(addprefix apps/, $(APPS))
arch_dir = arch/$(ARCH)
objs = apps.o
stub_objs =

ifneq ($(ARCH),)
    ifeq ($(TARGET), kernel)
        target = kernel
        include kernel/kernel.mk
    else ifeq ($(TARGET), user)
        target = user
    else
        $(error "TARGET must be 'kernel' or 'user'")
    endif

    -include $(BUILD_DIR)/apps.mk
    include $(addsuffix /lang.mk, $(addprefix lang/, $(LANGS)))
    _objs := $(objs)
    objs = $(addprefix $(BUILD_DIR)/, $(_objs)) \
           $(addprefix $(BUILD_DIR)/$(arch_dir)/, $(arch_objs)) \
           $(stub_objs)
    include arch/$(ARCH)/arch.mk
    -include $(objs:.o=.deps)
endif

.SUFFIXES:
.SECONDARY:
$(VERBOSE).SILENT:
.PHONY: build test clean

build: $(TARGET_FILE)

test:
	$(MAKE) ARCH=posix TARGET=kernel APPS=kernel-test \
		BUILD_DIR=ktest TARGET_FILE=ktest/ktest
	$(MAKE) run ARCH=posix TARGET=kernel TARGET_FILE=./ktest/ktest \
		BUILD_DIR=ktest

$(BUILD_DIR)/apps.mk: $(addsuffix /app.yaml, $(apps_dir))
	$(CMDECHO) GEN $@
	$(MKDIR) -p $(@D)
	PYTHONPATH=$(makefile_dir) ./tools/gen-apps-mk $^ > $@

$(BUILD_DIR)/apps.o: $(BUILD_DIR)/apps.c
$(BUILD_DIR)/apps.c: $(BUILD_DIR)/apps.mk
	$(CMDECHO) GEN $@
	$(MKDIR) -p $(@D)
	PYTHONPATH=$(makefile_dir) ./tools/gen-apps-c $(APPS) > $@
