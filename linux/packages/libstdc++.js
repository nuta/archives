const { config } = require('../pkgbuilder').pkg

const version = '7.2.0-8ubuntu3'

module.exports = {
  name: 'libstdc++',
  type: 'library',
  version,

  url() {
    return `${config('target.ubuntu_pkg_url')}/pool/main/g/gcc-7/libstdc++6_${version}_${config('target.deb_arch')}.deb`
  },

  sha256() {
    switch (config('target.deb_arch')) {
      case 'amd64': return '156fc7ca6bf91053b7dde818fa0e5ec1b49b725ff5922b19b44f8c90fc1d3e70'
      case 'armhf': return '9147db55ec6c9f97e95131cd5cf804a4c0b4f3bfbf18d65a392ebef7d3b020f4'
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
