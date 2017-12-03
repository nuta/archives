const { config } = require('../pkgbuilder').pkg

const version = '6.2.0-5ubuntu12'

module.exports = {
  name: 'libgcc',
  type: 'library',
  version,

  url() {
    return `${config('target.ubuntu_pkg_url')}/pool/main/g/gcc-6/libgcc1_${version}_${config('target.deb_arch')}.deb`
  },

  sha256() {
    switch (config('target.deb_arch')) {
      case 'amd64': return '56890482f7c86d31d5c3883dff0174bfa3a1822253d9cee75e6399e71425b61a'
      case 'armhf': return '6c4f79c1c10b11e93480b8a288e15f16196174ebf5c2f87d2a9503361defb067'
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
