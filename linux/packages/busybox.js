const { isNewerFile, copyFile, assetPath, run, config } = require('../pkgbuilder').pkg

const version = '1.27.2'
const url = `http://busybox.net/downloads/busybox-${version}.tar.bz2`

module.exports = {
  name: 'busybox',
  type: 'application',
  version,
  url,
  sha256: '9d4be516b61e6480f156b11eb42577a13529f75d3383850bb75c50c285de63df',

  changed() {
    return isNewerFile('.config', assetPath('busybox', 'config'))
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
    '/bin/busybox': 'busybox'
  }
}
