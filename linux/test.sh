#!/usr/bin/env zsh
set -e

(sleep 14;echo "\x1xq") | qemu-system-x86_64 -nographic -kernel kernel.img -append "console=ttyS0" -netdev user,id=net0 -device virtio-net,netdev=net0 -drive file=fat:rw:disk,if=virtio | tee log

grep log "*** userland testing tool" > /dev/null
grep log "*** success!" > /dev/null
