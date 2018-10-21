export PWD  := $(shell pwd)
export PATH := $(PWD)/deps/xtensa-esp32-elf/bin:$(PATH)
COMMANDS := build menuconfig clean flash monitor print_flash_cmd

export APP_OBJS FIRMWARE_VERSION

.PHONY: $(COMMANDS)
$(COMMANDS):
	$(MAKE) -f build.mk $@

$(V).SILENT:
