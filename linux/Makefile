$(V).SILENT:
.PHONY: default build prepare
default: build

REMOTE ?= local
REMOTE_PATH ?= makestack
TARGET ?= x64
BUILD ?= debug
VERSION ?= dev

pkgbuilder/node_modules:
	cd $(@D) && yarn

build: pkgbuilder/node_modules
	./pkgbuilder/pkgbuilder $(TARGET) $(VERSION)

remote-build:
	cd .. && ./tools/rsync-repo $(REMOTE) $(REMOTE_PATH)
	ssh -t $(REMOTE) 'cd $(REMOTE_PATH)/linux && make build TARGET=$(TARGET) VERSION=$(VERSION)'
