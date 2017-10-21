const { config, assetPath } = require('../pkgbuilder').pkg

const version = '2.24-11+deb9u1'

module.exports = {
  name: 'glibc',
  type: 'library',
  version,

  url() {
    return `http://http.us.debian.org/debian/pool/main/g/glibc/libc6_${version}_${config('target.deb_arch')}.deb`
  },

  sha256() {
    switch (config('target.deb_arch')) {
      case 'amd64': return 'b3f7278d80d5d0dc428fe92309bbc0e0a1ed665548a9f660663c1e1151335ae9'
      case 'armhf': return 'dd6bbae5d617a0e979c7ecfce7545c8ac9c50a827494adcc7985379d92d7a58c'
      default: throw new Error(`unknown target.deb_arch: \`${config('deb_arch')}'`)
    }
  },

  changed() {
    return false
  },

  build() {
  },

  rootfs() {
    const files = {}
    files[config('glibc.ldDestPath')] = config('glibc.ldSourcePath')
    return Object.assign(files, {
      '/etc/hosts': assetPath('glibc', 'hosts'),
      '/lib/libnss_nis.so.2': `lib/${config('target.libTriplet')}/libnss_nis.so.2`,
      '/lib/libm.so.6': `lib/${config('target.libTriplet')}/libm.so.6`,
      '/lib/libthread_db.so.1': `lib/${config('target.libTriplet')}/libthread_db.so.1`,
      '/lib/libresolv.so.2': `lib/${config('target.libTriplet')}/libresolv.so.2`,
      '/lib/libnss_nisplus.so.2': `lib/${config('target.libTriplet')}/libnss_nisplus.so.2`,
      '/lib/libanl.so.1': `lib/${config('target.libTriplet')}/libanl.so.1`,
      '/lib/libdl.so.2': `lib/${config('target.libTriplet')}/libdl.so.2`,
      '/lib/libc.so.6': `lib/${config('target.libTriplet')}/libc.so.6`,
      '/lib/libBrokenLocale.so.1': `lib/${config('target.libTriplet')}/libBrokenLocale.so.1`,
      '/lib/libnss_files.so.2': `lib/${config('target.libTriplet')}/libnss_files.so.2`,
      '/lib/libpthread.so.0': `lib/${config('target.libTriplet')}/libpthread.so.0`,
      '/lib/libcidn.so.1': `lib/${config('target.libTriplet')}/libcidn.so.1`,
      '/lib/libnss_hesiod.so.2': `lib/${config('target.libTriplet')}/libnss_hesiod.so.2`,
      '/lib/librt.so.1': `lib/${config('target.libTriplet')}/librt.so.1`,
      '/lib/libnss_compat.so.2': `lib/${config('target.libTriplet')}/libnss_compat.so.2`,
      '/lib/libnss_dns.so.2': `lib/${config('target.libTriplet')}/libnss_dns.so.2`,
      '/lib/libutil.so.1': `lib/${config('target.libTriplet')}/libutil.so.1`,
      '/lib/libcrypt.so.1': `lib/${config('target.libTriplet')}/libcrypt.so.1`,
      '/lib/libnsl.so.1': `lib/${config('target.libTriplet')}/libnsl.so.1`
    })
  }
}
