const { isNewerFile, copyFile, assetPath, run, config } = require('../pkgbuilder').pkg
const path = require('path')

const version = '2.6'
const url = `https://w1.fi/releases/wpa_supplicant-${version}.tar.gz`

module.exports = {
  name: 'wpa_supplicant',
  type: 'application',
  version,
  url,
  sha256: 'b4936d34c4e6cdd44954beba74296d964bc2c9668ecaa5255e499636fe2b1450',
  dependencies: ['openssl', 'libnl'],

  changed(buildDir) {
    return !buildDir || isNewerFile(
      assetPath('wpa_supplicant', 'config'),
      path.resolve(buildDir, 'wpa_supplicant/.config')
    )
  },

  build() {
    copyFile(assetPath('wpa_supplicant', 'config'), 'wpa_supplicant/.config')

    run(['make', 'wpa_supplicant'], {
      CC: `${config('target.toolchain_prefix')}gcc`,
      LD: `${config('target.toolchain_prefix')}ld`,
      CFLAGS: `-I${config('target.build_prefix')}/include/ -I${config('target.build_prefix')}/include/libnl3`,
      LIBS: `-L${config('target.build_prefix')}/lib`,
      LIBDIR: `${config('target.build_prefix') }/lib`,
      INCDIR: `${config('target.build_prefix')}/include`,
      BINDIR: `${config('target.build_prefix')}/sbin`,
      PKG_CONFIG_PATH: `${config('target.build_prefix')}/lib/pkgconfig`
    }, 'wpa_supplicant')
  },

  rootfs() {
    return {
      '/bin/wpa_supplicant': `wpa_supplicant/wpa_supplicant`
    }
  }
}
