#!/bin/sh
set -eu

exec /usr/bin/qemu-system-aarch64 \
  -cpu host -accel kvm -m 256 -machine virt,gic-version=2 \
  -kernel ftl.elf -nographic -serial mon:stdio --no-reboot \
  -sandbox on,obsolete=deny,elevateprivileges=deny,spawn=deny,resourcecontrol=deny \
  -global virtio-mmio.force-legacy=false \
  -device virtio-net-device,netdev=net0,bus=virtio-mmio-bus.0 \
  -netdev user,id=net0,restrict=on,hostfwd=tcp:0.0.0.0:30080-:80
