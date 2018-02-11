const { config } = require('../pkgbuilder').pkg

const version = '7.2.0-8ubuntu3.2'

module.exports = {
  name: 'libgcc',
  type: 'library',
  version,

  url() {
    console.log(`${config('target.ubuntu_pkg_url')}/pool/main/g/gcc-7/libgcc1_${version}_${config('target.deb_arch')}.deb`)
    return `${config('target.ubuntu_pkg_url')}/pool/main/g/gcc-7/libgcc1_${version}_${config('target.deb_arch')}.deb`
  },

  sha256() {
    switch (config('target.deb_arch')) {
      case 'amd64': return '0c2ab67c2a031d11696c4bc3dc18ea77c16aa7c14a5d3b6694a1a5fa70275122'
      case 'armhf': return 'b9a8237c6c2ed0497493e0be89326c82b947b9da083d9b3a75de6f680e76407c'
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
      '/lib/libgcc_s.so.1': `lib/${config('target.libTriplet')}/libgcc_s.so.1`
    }
  }
}
