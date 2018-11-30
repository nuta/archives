ifneq ($(shell test -d deps && echo yes), yes)
$(error Run ./tools/download-dependencies first!)
endif

DEPS_DIR :=	$(PWD)/deps

# Variables used by ESP-IDF.
PROJECT_NAME := firmware
EXTRA_COMPONENT_DIRS = \
	supervisor \
	jerryscript \
	$(DEPS_DIR)/arduino-esp32

# ESP-IDF expects V=1 or otherwise.
ifneq ($(V),)
	V=1
endif

IDF_PATH := $(PWD)/deps/esp-idf

.PHONY: build
build: all_binaries

$(V).SILENT:
-include $(IDF_PATH)/make/project.mk
