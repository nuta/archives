ifeq ($(BOARD),)
$(error $$BOARD is not set)
endif

export FIRMWARE_DIR := $(shell pwd)
export BOARD_DIR := $(FIRMWARE_DIR)/boards/$(BOARD)
export DEPS_DIR := $(FIRMWARE_DIR)/deps/esp32
export BUILD_DIR = $(FIRMWARE_DIR)/build/$(BOARD)

.PHONY: build
build:
$(V).SILENT:

include $(BOARD_DIR)/board.mk
