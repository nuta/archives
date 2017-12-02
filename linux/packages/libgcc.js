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
      case 'amd64': return '555d63f1d9f479db23f75a420401ea7298a684ce2acdb48e5d5eca6e22f54ac5'
      case 'armhf': return '888cfa54e273526a3d34c879dea91992954e6a822bec0702984930d30e98f7d2'
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
