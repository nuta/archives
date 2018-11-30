const { assetPath } = require('../pkgbuilder').pkg

module.exports = {
  name: 'init',

  changed() {
    return false
  },

  rootfs: {
    '/init': assetPath('init', 'init')
  }
}
