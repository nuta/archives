$(V).SILENT:
.PHONY: default build prepare
default: build

TARGET ?= x64
BUILD ?= debug
VERSION ?= dev

prepare:
	docker build -t makestack/linux .

x64:
	$(MAKE) TARGET=x64

raspberrypi3:
	$(MAKE) TARGET=raspberrypi3

ifeq ($(DOCKER),)

build:
	./pkgbuilder/pkgbuilder $(TARGET) $(VERSION)

else

pkgbuilder/node_modules:
	cd $(@D) && yarn

build: pkgbuilder/node_modules
	docker run --rm \
	  -e TARGET=$(TARGET) \
	  -e MAKEFLAGS="-j$(shell getconf _NPROCESSORS_ONLN)" \
	  -e BUILD=$(BUILD) \
	  -e VERSION=$(VERSION) \
	  -v $(shell realpath ..):/makestack -it makestack/linux

endif
