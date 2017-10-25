const { config } = require('../pkgbuilder').pkg

const version = '8.2.1'

module.exports = {
  name: 'node',
  version,

  url() {
    const arch = config('target.node_arch')
    const url = `https://nodejs.org/dist/v${version}/node-v${version}-linux-${arch}.tar.xz`
    return url
  },

  sha256() {
    switch (config('target.node_arch')) {
      case 'x64': return 'abcddeb95cc4465953b1edb0922d20e9b0b3de83688fc8150b863117032a978a'
      case 'armv7l': return '52a3abbf9450fbb91b39aaa268977a593daa8fc7dec0925d32ac7afd3a3596e2'
      default: throw new Error(`unknown target.node_arch: \`${config('node_arch')}'`)
    }
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
