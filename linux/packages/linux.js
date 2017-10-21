const { isNewerFile, copyFile, isRebuilt, assetPath, run, config } = require('../pkgbuilder').pkg

module.exports = {
  name: 'linux',
  type: 'kernel',
  dependencies: ['initramfs'],
  version() {
    return config('linux.version')
  },

  url() {
    return config('linux.url')
  },

  sha256() {
    return config('linux.sha256')
  },

  changed() {
    return isRebuilt('initramfs') ||
      isNewerFile('.config',
          assetPath(config('target.name'), 'linux.config'))
  },

  build() {
    const cpioPath = config('initramfs.cpio_path')

    copyFile(assetPath(config('target.name'), 'linux.config'), '.config')
    run(['sed', '-i', `s#__INITRAMFS_CPIO_PATH__#${cpioPath}#`, '.config'])

    run(['make', config('linux.make_target')], {
      ARCH: config('target.linux_arch'),
      CROSS_COMPILE: config('target.toolchain_prefix')
    })
  },

  bootfs() {
    return config('linux.bootfs')
  },

  rootfs() {
    return config('linux.rootfs')
  }
}
