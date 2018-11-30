const { config } = require('../pkgbuilder').pkg

const version = '8.9.3'

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
      case 'x64': return '86f3aa593315f0503d069e3f4805019583ab8d86c0244a83c795d1942e3f99b7'
      case 'armv7l': return '4ca2b9a5b46c22353233365944c2df7f1566a298bf25f57be8c863bf1bd4e0f8'
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
