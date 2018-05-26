default: build
include mk/common.mk
include kernel/kernel.mk

server_dirs := $(wildcard servers/*)
.PHONY: default build clean run test $(server_dirs)

build: $(BUILD_DIR)/kernel/kernel.elf

$(server_dirs):
	$(PROGRESS) MAKE $@
	$(MAKE) -f mk/server.mk build DIR=$@ BUILD_DIR=$(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)
