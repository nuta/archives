default: build
include common.mk
include kernel/kernel.mk

server_dirs := $(wildcard servers/*)
.PHONY: default build stubs clean run test $(server_dirs)

STUBS_DIR := $(BUILD_DIR)/stubs
stubs := $(wildcard interfaces/*.idl)
tools/idl/parser/idlParser.py: tools/idl/idl.g4
	mkdir -p tools/idl/parser
	$(PROGRESS) ANTLR4 tools/idl/parser
	cd tools/idl && \
		$(ANTLR4) -Dlanguage=Python3 -o parser $(notdir $<)
	touch $(dir $@)__init__.py

stubs: tools/idl/parser/idlParser.py tools/genstub.py $(stubs)
	$(PROGRESS) "GENSTUB" $(STUBS_DIR)/c
	./tools/genstub.py --out-dir $(STUBS_DIR)/c --lang c $(stubs)
	$(PROGRESS) "GENSTUB" libs/resea/src/interfaces
	./tools/genstub.py --out-dir libs/resea/src/interfaces --lang rust $(stubs)

build:
	@mkdir -p $(KFS_DIR)/servers
	@set -e; for server_dir in $(addprefix servers/, $(SERVERS)); do \
		$(MAKE) $$server_dir; \
	done
	$(MAKE) $(BUILD_DIR)/kernel/kernel.elf

$(server_dirs):
	$(PROGRESS) MAKE $@
	$(MAKE) -f servers/server.mk build DIR=$@ BUILD_DIR=$(BUILD_DIR) KFS_DIR=$(KFS_DIR)

clean:
	rm -rf $(BUILD_DIR)
