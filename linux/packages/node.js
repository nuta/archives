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

  sha256() {
    switch (config('target.node_arch')) {
      case 'x64': return '74e125920e141357bab6ded58198c5f5ee98bf638aaff474ffb355ab699d6a26'
      case 'armv7l': return '35bcad5396e61311ee7feec8eeeeaba7675a1660081a10751c851c831d3a66cb'
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
