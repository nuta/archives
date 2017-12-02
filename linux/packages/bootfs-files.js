const { assetPath } = require('../pkgbuilder').pkg

module.exports = {
  name: 'bootfs-files',

  bootfs: {
    '/makestack.config': assetPath('bootfs-files', 'makestack.config')
  }
}
