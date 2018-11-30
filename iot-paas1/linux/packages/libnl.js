const fs = require('fs')
const { assetPath, run, config } = require('../pkgbuilder').pkg

const version = '3.4.0'
const url = `https://github.com/thom311/libnl/releases/download/libnl${version.replace(/\./g, '_')}/libnl-${version}.tar.gz`

module.exports = {
  name: 'libnl',
  type: 'library',
  version,
  url,
  sha256: 'b7287637ae71c6db6f89e1422c995f0407ff2fe50cecd61a312b6a9b0921f5bf',
  dependencies: [],

  changed() {
  },

  build() {
    run(['./configure', `--prefix=${config('target.build_prefix')}`,
      `--host=${config('target.configure_host')}`])

    run(['make'])
    run(['make', 'install'])
  },

  rootfs() {
    return {
      '/lib/libnl-3.so.200': `${config('target.build_prefix')}/lib/libnl-3.so.200`,
      '/lib/libnl-genl-3.so.200': `${config('target.build_prefix')}/lib/libnl-genl-3.so.200`
    }
  }
}
