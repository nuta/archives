const { run, config } = require('../pkgbuilder').pkg

const version = '1.1.0f'
const url = `https://openssl.org/source/openssl-${version}.tar.gz`

module.exports = {
  name: 'openssl',
  type: 'library',
  version,
  url,
  sha256: '12f746f3f2493b2f39da7ecf63d7ee19c6ac9ec6a4fcd8c229da8a522cb12765',
  dependencies: [],

  changed() {
    return false
  },

  build() {
    run([`./Configure`, config('openssl.configure_target'),
         'no-ssl3', 'no-comp', '-DOPENSSL_NO_HEARTBEATS',
         `--prefix=${config('target.build_prefix')}`], {
      CC: `${config('target.toolchain_prefix')}gcc`,
      LD: `${config('target.toolchain_prefix')}ld`
    })

    run(['make'], {
      CC: `${config('target.toolchain_prefix')}gcc`,
      LD: `${config('target.toolchain_prefix')}ld`
    })

    run(['make', 'install_runtime', 'install_dev'])
  },

  rootfs() {
    return {
      '/lib/libcrypto.so.1.1': `${config('target.build_prefix')}/lib/libcrypto.so.1.1`,
      '/lib/libssl.so.1.1': `${config('target.build_prefix')}/lib/libssl.so.1.1`
    }
  }
}
