const fs = require('fs')

module.exports = {
  name: 'rootfs-files',

  build() {
    fs.writeFileSync('VERSION', process.env.VERSION)
  },

  rootfs: {
    '/VERSION': 'VERSION'
  }
}
