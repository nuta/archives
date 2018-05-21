const { config, assetPath, defineUbuntuPackage } = require('../pkgbuilder').pkg

module.exports = defineUbuntuPackage('libc6', {
  name: 'glibc',
  type: 'library',
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
})
