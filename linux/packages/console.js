const { sudo, rootfsPath, mkdirp } = require('../pkgbuilder').pkg

module.exports = {
  name: 'console',

  changed() {
    return false
  },

  build() {
    mkdirp(rootfsPath('dev'))
    sudo(['mknod', rootfsPath('dev/console'), 'c', '5', '1'])
  },

  rootfs: {
    '/dev/console': false
  }
}
