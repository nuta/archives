const { isRebuilt, run, config, rootfsPath, buildPath } = require('../pkgbuilder').pkg

const commonDependencies = [
  'rootfs-files',
  'init',
  'node',
  'supervisor',
  'busybox',
  'apparmor',
  'wpa_supplicant',
  'iw',
  'glibc',
  'libstdc++',
  'libgcc'
]

module.exports = {
  name: 'initramfs',
  type: 'diskimage',
  dependencies() {
    return commonDependencies.concat(config('target.initramfs_dependencies'))
  },

  config() {
    return {
      cpio_path: buildPath('initramfs.cpio')
    }
  },

  changed() {
    return commonDependencies.concat(config('target.initramfs_dependencies')).some(isRebuilt)
  },

  build() {
    process.chdir(rootfsPath('.'))
    run(['sh', '-c', `find . | cpio -ov --format=newc > ${config('initramfs.cpio_path')}`])
  }
}
