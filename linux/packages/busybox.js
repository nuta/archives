const { isNewerFile, copyFile, assetPath, run, config } = require('../pkgbuilder').pkg

const version = '1.27.2'
const url = `http://busybox.net/downloads/busybox-${version}.tar.bz2`

module.exports = {
  name: 'busybox',
  type: 'application',
  version,
  url,

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
