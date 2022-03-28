ifneq ($(shell test -d $(FIRMWARE_DIR)/deps/esp32 && echo no), no)
$(error Depdendencies are not installed!)
endif

export PATH := $(DEPS_DIR)/xtensa-esp32-elf/bin:$(PATH)
COMMANDS := menuconfig clean flash monitor print_flash_cmd erase_flash

.PHONY: build $(COMMANDS)
$(COMMANDS):
	$(MAKE) -f $(BOARD_DIR)/esp_idf.mk $@

ADAPTER ?= serial
build:
	$(MAKE) -f $(BOARD_DIR)/esp_idf.mk all_binaries
	echo Embedding the credential...
	WIFI_PASSWORD="$(WIFI_PASSWORD)" ./embed-cred.py \
		--version $(shell date "+%s") \
		--adapter "$(ADAPTER)" \
		--wifi-ssid "$(WIFI_SSID)" \
		--server-url "$(SERVER_URL)" \
		$(BUILD_DIR)/firmware.elf
	rm -f $(BUILD_DIR)/firmware.bin
	$(MAKE) -f $(BOARD_DIR)/esp_idf.mk build_bin
