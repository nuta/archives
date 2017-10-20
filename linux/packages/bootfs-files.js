const fs = require('fs')
const { assetPath } = require('../pkgbuilder').pkg

module.exports = {
  name: 'bootfs-files',

  bootfs: {
    '/config.sh': assetPath('bootfs-files', 'config.sh')
  }
}
