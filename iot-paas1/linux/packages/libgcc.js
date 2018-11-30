const { config, defineUbuntuPackage } = require('../pkgbuilder').pkg

module.exports = defineUbuntuPackage('libgcc1', {
  name: 'libgcc',
  type: 'library',
  rootfs: {
    '/lib/libgcc_s.so.1': `lib/${config('target.libTriplet')}/libgcc_s.so.1`
  }
})
