export PWD  := $(shell pwd)
export PATH := $(PWD)/deps/xtensa-esp32-elf/bin:$(PATH)
COMMANDS := build menuconfig clean flash monitor

export APP_OBJS DEVICE_ID WIFI_SSID WIFI_PASSWORD SERVER_URL CA_CERT
export FIRMWARE_VERSION APP_VERSION

.PHONY: $(COMMANDS)
$(COMMANDS):
	$(MAKE) -f build.mk $@

$(V).SILENT:
