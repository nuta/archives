const { isNewerFile, copyFile, assetPath, run, config } = require('../pkgbuilder').pkg

const version = '4.9'
const url = `https://www.kernel.org/pub/software/network/iw/iw-${version}.tar.xz`

module.exports = {
  name: 'iw',
  type: 'application',
  version,
  url,
  sha256: '324cc805fad52cba2c16b9ab569906889fb645cc962aac4cfda1db85d2de97ce',
  dependencies: ['libnl'],

  changed() {
  },

  build() {
    run(['make'], {
      CC: `${config('target.toolchain_prefix')}gcc`,
      LD: `${config('target.toolchain_prefix')}ld`,
      PREFIX: config('target.build_prefix'),
      PKG_CONFIG_PATH: `${config('target.build_prefix')}/lib/pkgconfig`,
      LDFLAGS: `-L${config('target.build_prefix')}/lib`
    })

    run(['make', 'install'], {
      PREFIX: config('target.build_prefix')
    })
  },

  rootfs() {
    return {
      '/bin/iw': `${config('target.build_prefix')}/sbin/iw`
    }
  }
}
