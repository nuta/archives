const { config, defineUbuntuPackage } = require('../pkgbuilder').pkg

module.exports = defineUbuntuPackage('libstdc++6', {
  name: 'libstdc++',
  type: 'library',
  rootfs: {
    '/lib/libstdc++.so.6': `usr/lib/${config('target.libTriplet')}/libstdc++.so.6`
  }
})
