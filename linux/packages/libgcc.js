const { config } = require('../pkgbuilder').pkg

const version = '6.3.0-18'

module.exports = {
  name: 'libgcc',
  type: 'library',
  version,

  url() {
    return `http://http.us.debian.org/debian/pool/main/g/gcc-6/libgcc1_${version}_${config('target.deb_arch')}.deb`
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
