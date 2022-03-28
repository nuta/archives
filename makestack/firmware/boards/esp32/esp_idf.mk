export IDF_PATH := $(DEPS_DIR)/esp-idf
PROJECT_NAME := firmware
EXTRA_COMPONENT_DIRS := $(BOARD_DIR)/firmware $(DEPS_DIR)/arduino-esp32 $(MY_COMPONENT_DIRS)
BUILD_DIR_BASE := $(BUILD_DIR)
CPPFLAGS += -I$(FIRMWARE_DIR)/include

build_elf: $(BUILD_DIR)/firmware.elf
build_bin: $(BUILD_DIR)/firmware.bin
include $(IDF_PATH)/make/project.mk
