#!/bin/sh
fakeroot make-kpkg --jobs `getconf _NPROCESSORS_ONLN` --initrd --append-to-version=-homu kernel_image kernel_headers
