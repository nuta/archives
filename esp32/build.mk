ifneq ($(shell test -d deps && echo yes), yes)
$(error Run ./tools/download-dependencies first!)
endif

ESP32_DIR := $(PWD)
DEPS_DIR := $(PWD)/deps
IDF_PATH := $(DEPS_DIR)/esp-idf

# Variables used by ESP-IDF.
PROJECT_NAME := firmware
EXTRA_COMPONENT_DIRS = \
	src \
	$(DEPS_DIR)/arduino-esp32 \
    $(MY_COMPONENTS)

ifeq ($(RELEASE),)
CPPFLAGS += -DDEBUG_BUILD -I$(ESP32_DIR)/src/include
endif

ifneq ($(V),)
	V=1
endif

.PHONY: build
build: all_binaries

$(V).SILENT:
-include $(IDF_PATH)/make/project.mk
