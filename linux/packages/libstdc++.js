const { config } = require('../pkgbuilder').pkg

const version = '7.2.0-18ubuntu2'

module.exports = {
  name: 'libstdc++',
  type: 'library',
  version,

  url() {
    return `${config('target.ubuntu_pkg_url')}/pool/main/g/gcc-7/libstdc++6_${version}_${config('target.deb_arch')}.deb`
  },

  sha256() {
    switch (config('target.deb_arch')) {
      case 'amd64': return 'dea90151dd5ac0626cde33ab455cfcac553ca3fe4a4438cfba5c02308bde359a'
      case 'armhf': return '2204858ca2bee2788162d80fa130b6443d15b29f13543b48636f1bf802190c01'
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
