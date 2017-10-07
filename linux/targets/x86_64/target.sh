NODE_ARCH=x64
DEB_ARCH=amd64
LD_ARCH=x86-64
TRIPLET=x86_64-linux-gnu
LINUX_VERSION=4.9.53
LINUX_TARBALL=$DOWNLOADS_DIR/linux-$LINUX_VERSION.tar.xz
LINUX_DIR=$BUILD_DIR/linux-$LINUX_VERSION
LINUX_MAKE_TARGET=bzImage
DYLINKER_PATH=lib64/ld-linux-${LD_ARCH}.so.2
VMLINUZ=$LINUX_DIR/arch/x86/boot/bzImage
DOWNLOAD_URLS=(\
    https://cdn.kernel.org/pub/linux/kernel/v4.x/linux-${LINUX_VERSION}.tar.xz \
    http://busybox.net/downloads/busybox-${BUSYBOX_VERSION}.tar.bz2 \
    https://nodejs.org/dist/v${NODE_VERSION}/node-v${NODE_VERSION}-linux-${NODE_ARCH}.tar.xz \
    http://http.us.debian.org/debian/pool/main/g/gcc-6/libgcc1_${LIBGCC_VERSION}_${DEB_ARCH}.deb \
    http://http.us.debian.org/debian/pool/main/g/gcc-6/libstdc++6_${LIBSTDCPP_VERSION}_${DEB_ARCH}.deb \
    http://http.us.debian.org/debian/pool/main/g/glibc/libc6_${GLIBC_VERSION}_${DEB_ARCH}.deb
    http://us.archive.ubuntu.com/ubuntu/pool/main/a/apparmor/apparmor_${APPARMOR_VERSION}_${DEB_ARCH}.deb
)

LIB_FILES=(\
        libgcc_s.so.1 \
        libnss_nis.so.2 \
        libm.so.6 \
        libmvec.so.1 \
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


test() {
    QEMU=(
      qemu-system-x86_64
      -kernel $BUILD_DIR/kernel.img
      -append '"console=ttyS0 root=/dev/vda1"'
      -netdev user,id=net0 -device virtio-net,netdev=net0
      -drive file=fat:rw:$DISK_DIR,if=virtio
      -nographic
    )

    zsh -c "(sleep 32 ; echo \"\\x1xq\") | $QEMU" | tee test.log
    grep "*** success" test.log
}
