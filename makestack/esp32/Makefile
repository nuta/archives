export PATH := $(PWD)/deps/xtensa-esp32-elf/bin:$(PATH)
COMMANDS := build menuconfig clean flash monitor

.PHONY: $(COMMANDS)
$(COMMANDS):
	$(MAKE) -f build.mk $@

$(V).SILENT:
