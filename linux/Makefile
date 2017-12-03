$(V).SILENT:
.PHONY: default build prepare
default: build

TARGET ?= x64
BUILD ?= debug

prepare:
	docker build -t makestack/linux .

x64:
	$(MAKE) TARGET=x64

raspberrypi3:
	$(MAKE) TARGET=raspberrypi3

ifeq ($(DOCKER),)

build:
	docker run --rm \
	  -e TARGET=$(TARGET) \
	  -e MAKEFLAGS="-j$(shell getconf _NPROCESSORS_ONLN)" \
	  -e BUILD=$(BUILD) \
	  -v $(shell realpath ..):/makestack -it makestack/linux

else

build:
	./pkgbuilder/pkgbuilder $TARGET

endif
