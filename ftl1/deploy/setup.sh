set -e

ARCH=$(uname -m)
if [ "$ARCH" != "aarch64" ]; then
    echo "Error: $ARCH machine is not supported yet. Prepare aarch64 machine instead."
    exit 1
fi

if ! command  -v qemu-system-aarch64 >/dev/null; then
    echo "Error: qemu-system-aarch64 is not installed."
    exit 1
fi

if [ ! -c /dev/kvm ]; then
    echo "Error: KVM is not available on this machine."
    exit 1
fi

sudo chmod +x /opt/ftl/boot.sh
sudo cp /opt/ftl/ftl.service /etc/systemd/system/ftl.service
sudo systemctl daemon-reload
sudo systemctl restart ftl
sudo systemctl enable ftl
