const { isRebuilt, run, config, rootfsPath, buildPath } = require('../pkgbuilder').pkg

const dependencies = ['busybox', 'apparmor', 'glibc', 'libstdc++', 'libgcc', 'node', 'supervisor', 'init']

module.exports = {
  name: 'initramfs',
  type: 'diskimage',
  dependencies,

  config() {
    return {
      cpio_path: buildPath('initramfs.cpio')
    }
  },

  changed() {
    return dependencies.some(isRebuilt)
  },

  build() {
    process.chdir(rootfsPath('.'))
    run(['sh', '-c', `find . | cpio -ov --format=newc > ${config('initramfs.cpio_path')}`])
  },
}
