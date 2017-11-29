$(V).SILENT:
.PHONY: default build prepare
default: build

TARGET ?= x64
BUILD ?= debug

prepare:
	docker build -t makestack/linux .

build:
	docker run --rm \
	  -e TARGET=$(TARGET) \
	  -e MAKEFLAGS="-j$(shell getconf _NPROCESSORS_ONLN)" \
	  -e BUILD=$(BUILD) \
	  -v $(shell realpath ..):/makestack -it makestack/linux
