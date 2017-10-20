$(V).SILENT:

.PHONY: default kernel yarn
default: kernel

kernel:
	$(MAKE) yarn
	./pkgbuilder/pkgbuilder $(TARGET)

yarn: pkgbuilder/node_modules

pkgbuilder/node_modules:
	cd pkgbuilder && yarn
