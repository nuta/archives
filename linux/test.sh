#!/usr/bin/env zsh
set -e

cleanup() { rm log }
trap cleanup EXIT

set -v
(sleep 32 ; echo "\x1xq") | qemu-system-x86_64 -nographic -kernel kernel.img -append "console=ttyS0 debug" -netdev user,id=net0 -device virtio-net,netdev=net0 -drive file=fat:rw:disk,if=virtio | tee log

grep "*** userland testing tool" log > /dev/null
grep "*** success!" log > /dev/null
