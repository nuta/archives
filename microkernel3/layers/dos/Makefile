export ARCH = x64
LAYER = dos
LAYER_DIR = layers/dos
SERVERS = pc
KFS_DIR = build/kernel/kfs

.PHONY: build bin run run-gui bochs

build:
	$(MAKE) -f $(LAYER_DIR)/Makefile bin
	$(MAKE) SERVERS="$(SERVERS)" LAYER=$(LAYER) MBRBOOT=$(MBRBOOT) build

bin:
	@mkdir -p $(KFS_DIR)/bin
	@set -e; for file in $(patsubst %.c, %, $(wildcard $(LAYER_DIR)/bin/*.c)); do \
		$(MAKE) -f $(LAYER_DIR)/lib/bin.mk LAYER_DIR=$(LAYER_DIR) TARGET=$(KFS_DIR)/bin/$$(basename $$file) SRC=$${file}.c; \
	done

run:
	$(MAKE) -f $(LAYER_DIR)/Makefile build
	$(MAKE) SERVERS="$(SERVERS)" LAYER=$(LAYER) run

bochs:
	$(MAKE) -f $(LAYER_DIR)/Makefile build MBRBOOT=1
	$(MAKE) SERVERS="$(SERVERS)" LAYER=$(LAYER) bochs

run-gui:
	$(MAKE) -f $(LAYER_DIR)/Makefile build
	$(MAKE) SERVERS="$(SERVERS)" LAYER=$(LAYER) run-gui
