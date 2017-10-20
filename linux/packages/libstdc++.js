const { config } = require('../pkgbuilder').pkg

const version = '6.3.0-18'

module.exports = {
  name: 'libstdc++',
  type: 'library',
  version,

  url() {
    return `http://http.us.debian.org/debian/pool/main/g/gcc-6/libstdc++6_${version}_${config('target.deb_arch')}.deb`
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
