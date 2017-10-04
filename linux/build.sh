#!/bin/bash
set -ue
TARGET=$PWD/x86_64
LINUX_VERSION=4.13.4
MUSL_VERSION=1.1.16
BUSYBOX_VERSION=1.27.2
NODE_VERSION=
BUILD_TOOLCHAIN_DIR=$PWD/build/usr

export MAKEFLAGS="-j $(getconf _NPROCESSORS_ONLN)"

download_and_extract() {
    url=$1
    tarball=$(basename $url)
    dirname=$(basename $tarball .tar.${tarball##*.})
    if [ ! -d "$dirname" ]; then
        if [ ! -f downloads/$tarball ]; then
            mkdir -p downloads
            cd downloads
            wget $1
            cd ..
        fi

        tar xf downloads/$tarball
    fi
}

download_and_extract_deb() {
    url=$1
    deb=$(basename $url)
    dirname=$(basename $deb .deb)
    if [ ! -d "$dirname" ]; then
        if [ ! -f downloads/$deb ]; then
            mkdir -p downloads
            cd downloads
            wget $1
            cd ..
        fi

        mkdir -p $dirname
        cd $dirname
        ar x ../downloads/$deb
        tar xf data.tar.xz
        cd ..
    fi
}

create_symlink() {
    [ -f $2 -o -d $2 ] || ln -s $1 $2
}

download_and_extract https://cdn.kernel.org/pub/linux/kernel/v4.x/linux-${LINUX_VERSION}.tar.xz
download_and_extract http://busybox.net/downloads/busybox-${BUSYBOX_VERSION}.tar.bz2
download_and_extract https://nodejs.org/dist/v6.11.4/node-v6.11.4-linux-x64.tar.xz
download_and_extract_deb http://http.us.debian.org/debian/pool/main/g/glibc/libc6_*-11+deb9u1_amd64.deb
download_and_extract_deb http://http.us.debian.org/debian/pool/main/g/gcc-6/libgcc1_6.3.0-18_amd64.deb
download_and_extract_deb http://http.us.debian.org/debian/pool/main/g/gcc-6/libstdc++6_6.3.0-18_amd64.deb

if [ ! -f linux-${LINUX_VERSION}/.config ]; then
    cp ${TARGET}/linux.config linux-${LINUX_VERSION}/.config
    sed -i "s#__INITRAMFS_DIR__#$PWD#" linux-${LINUX_VERSION}/.config
fi

if [ ! -f busybox-${BUSYBOX_VERSION}/.config ]; then
    cp ${TARGET}/busybox.config busybox-${BUSYBOX_VERSION}/.config
fi

cd busybox-${BUSYBOX_VERSION}
make busybox
cd ..

cd initramfs
mkdir -p lib64
cp -n ../libc*/lib/*/ld-*.so lib64/ld-linux-x86-64.so.2
cp -n ../libc*/lib/*/lib*.so.* lib
cp -n ../libgcc*/lib/*/libgcc_s.so.1 lib/libgcc_s.so.1
cp -n ../libstdc++*/usr/lib/*/libstdc++.so.*.* lib/libstdc++.so.6

cp ../busybox-${BUSYBOX_VERSION}/busybox ../initramfs/bin
[ -e dev/console ] || sudo mknod dev/console c 5 1

find . | cpio -ov --format=newc > ../initramfs.cpio
cd ..

cd linux-${LINUX_VERSION}
make -j4 bzImage
cd ..

cp linux-${LINUX_VERSION}/arch/x86/boot/bzImage kernel.img
echo "Successfully built MakeStack Linux!"
du -h kernel.img
sha256sum kernel.img