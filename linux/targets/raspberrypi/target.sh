NODE_ARCH=armv7l
DEB_ARCH=armhf
LD_ARCH=armhf
TRIPLET=arm-linux-gnueabihf
RPI_LINUX_VERSION=1.20170811-1
LINUX_DIR=$BUILD_DIR/linux-raspberrypi-kernel_${RPI_LINUX_VERSION}
LINUX_TARBALL=$DOWNLOADS_DIR/raspberrypi-kernel_${RPI_LINUX_VERSION}.tar.gz
LINUX_MAKE_TARGET=zImage
DYLINKER_PATH=lib/ld-2.24.so
VMLINUZ=$LINUX_DIR/arch/arm/boot/zImage
DOWNLOAD_URLS=(\
    https://github.com/raspberrypi/linux/archive/raspberrypi-kernel_${RPI_LINUX_VERSION}.tar.gz \
    http://busybox.net/downloads/busybox-${BUSYBOX_VERSION}.tar.bz2 \
    https://nodejs.org/dist/v${NODE_VERSION}/node-v${NODE_VERSION}-linux-${NODE_ARCH}.tar.xz \
    http://http.us.debian.org/debian/pool/main/g/gcc-6/libgcc1_${LIBGCC_VERSION}_${DEB_ARCH}.deb \
    http://http.us.debian.org/debian/pool/main/g/gcc-6/libstdc++6_${LIBSTDCPP_VERSION}_${DEB_ARCH}.deb \
    http://http.us.debian.org/debian/pool/main/g/glibc/libc6_${GLIBC_VERSION}_${DEB_ARCH}.deb
)

LIB_FILES=(\
        libgcc_s.so.1 \
        libnss_nis.so.2 \
        libm.so.6 \
        libthread_db.so.1 \
        libresolv.so.2 \
        libnss_nisplus.so.2 \
        libanl.so.1 \
        libdl.so.2 \
        libc.so.6 \
        libBrokenLocale.so.1 \
        libnss_files.so.2 \
        libpthread.so.0 \
        libcidn.so.1 \
        libnss_hesiod.so.2 \
        librt.so.1 \
        libnss_compat.so.2 \
        libnss_dns.so.2 \
        libutil.so.1 \
        libcrypt.so.1 \
        libnsl.so.1
)

# For Busybox and Linux.
export ARCH=arm
export CROSS_COMPILE=arm-linux-gnueabihf- 
export KERNEL=kernel


test() {
    QEMU=(
	  qemu-system-arm -machine virt
      -kernel $BUILD_DIR/kernel.img
      -append '"console=ttyS0 root=/dev/vda1"'
      -netdev user,id=net0 -device virtio-net,netdev=net0
      -drive file=fat:rw:$DISK_DIR,if=virtio
      -nographic
	)

    zsh -c "(sleep 32 ; echo \"\\x1xq\") | $QEMU"
}
