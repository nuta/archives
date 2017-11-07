const { config } = require('../pkgbuilder').pkg

const version = '8.9.0'

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
      case 'x64': return 'e92b91fa473f9ad805a1241907b6f1bd3f8ceac8426a8b4cb05428e62e243bdd'
      case 'armv7l': return '36edb836120a68ab9a660e869e5ca3073f5cee880621d9ea4233d671632c33f5'
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
