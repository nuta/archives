BUILD_DIR ?= build
include $(BUILD_DIR)/config.mk

.SUFFIXES:
.SECONDARY:
$(VERBOSE).SILENT:

VERBOSE ?=
CMDECHO ?= printf "  \033[1;35m%7s    \033[0;33m%s\033[m\n"
MKDIR   ?= mkdir

makefile_dir = $(PWD)
arch_dir = arch/$(ARCH)
objs += apps.o

include $(addsuffix /lang.mk, $(addprefix lang/, $(LANGS)))

interfaces := $(sort $(interfaces))
objs := $(addprefix $(BUILD_DIR)/, $(objs)) $(stub_objs)

include arch/$(ARCH)/arch.mk

.PHONY: build
build: $(TARGET_FILE)


$(BUILD_DIR)/apps.o: $(BUILD_DIR)/apps.c
$(BUILD_DIR)/apps.c: $(BUILD_DIR)/config.mk
	$(CMDECHO) GEN $@
	$(MKDIR) -p $(@D)
	PYTHONPATH=$(makefile_dir) ./tools/gen-apps-c $(APPS) > $@


-include $(objs:.o=.deps)
