const { config } = require('../pkgbuilder').pkg

const version = '6.3.0-12ubuntu2'

module.exports = {
  name: 'libstdc++',
  type: 'library',
  version,

  url() {
    return `${config('target.ubuntu_pkg_url')}/pool/main/g/gcc-6/libstdc++6_${version}_${config('target.deb_arch')}.deb`
  },

  sha256() {
    switch (config('target.deb_arch')) {
      case 'amd64': return '0137c85ecbb1828c1e6cef0285f703c7bb2d78786f3e6173e55823c60a07a0bf'
      case 'armhf': return '53ace39a04ef00d5a5af747a9165a22863414e6f86497f7b774cab042633c304'
      default: throw new Error(`unknown target.deb_arch: \`${config('deb_arch')}'`)
    }
  },

  changed() {
    return false
  },

  build() {
  },

  rootfs() {
    return {
      '/lib/libstdc++.so.6': `usr/lib/${config('target.libTriplet')}/libstdc++.so.6`
    }
  }
}
