const { config } = require('../pkgbuilder').pkg

const version = '6_6.3.0-12ubuntu2'

module.exports = {
  name: 'libstdc++',
  type: 'library',
  version,

  url() {
    return `${config('target.ubuntu_pkg_url')}/pool/main/g/gcc-6/libstdc++6_${version}_${config('target.deb_arch')}.deb`
  },

  sha256() {
    switch (config('target.deb_arch')) {
      case 'amd64': return '7d583f9d6b83de18dc6352d0e3ee8e0af9ab9dc2ddcf3e78a5c4380fcb77da3d'
      case 'armhf': return 'b15930518cbd5ffa53864e5f51ec307e15110ffee920e9c697dfd3e79456faad'
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
