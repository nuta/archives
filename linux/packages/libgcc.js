const { config } = require('../pkgbuilder').pkg

const version = '7.3.0-1ubuntu1'

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
      case 'amd64': return '2edb81813e5131ac933d44076703d667cf755391a5870aeeb5fe186788a56e25'
      case 'armhf': return '8bd1f776467bbad6de3eaebfdfafc401d0d7cdce555cee1b296fbac47259ca39'
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
