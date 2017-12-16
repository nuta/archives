const fs = require('fs')

module.exports = {
  name: 'rootfs-files',

  build() {
    fs.writeFileSync('VERSION', build.version)
  },

  rootfs: {
    '/VERSION': 'VERSION'
  }
}
