const { config } = require('../pkgbuilder').pkg

const version = '6.11.4'

module.exports = {
  name: 'node',
  version,

  url() {
    const arch = config('target.node_arch')
    const url = `https://nodejs.org/dist/v${version}/node-v${version}-linux-${arch}.tar.xz`
    return url
  },

  changed() {
    return false
  },

  build() {
  },

  rootfs: {
    '/bin/node': 'bin/node'
  }
}
