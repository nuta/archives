const { isNewerFile, copyFile, assetPath, run, config } = require('../pkgbuilder').pkg
const path = require('path')

const version = '1.27.2'
const url = `http://busybox.net/downloads/busybox-${version}.tar.bz2`

module.exports = {
  name: 'busybox',
  type: 'application',
  version,
  url,
  sha256: '9d4be516b61e6480f156b11eb42577a13529f75d3383850bb75c50c285de63df',

  changed(buildDir) {
    return !buildDir || isNewerFile(assetPath('busybox', 'config'), path.resolve(buildDir, '.config'))
  },

  build() {
    copyFile(assetPath('busybox', 'config'), '.config')
    run(['make'], {
      ARCH: config('target.linux_arch'),
      CROSS_COMPILE: config('target.toolchain_prefix')
    })
  },

  rootfs: {
    '/etc/udhcpc.script': assetPath('busybox', 'udhcpc.script'),
    '/etc/mdev.conf': assetPath('busybox', 'mdev.conf'),
    '/etc/syslog.conf': assetPath('busybox', 'syslog.conf'),
    '/bin/busybox': 'busybox'
  }
}
