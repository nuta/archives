const { spawnSync } = require('child_process')
const {
   isRebuilt, bootfsPath, buildPath, run, sudo, buildFatImage
} = require('../pkgbuilder').pkg

const version = '4.9.53'
const dependencies = ['linux', 'bootfs-files']

module.exports = {
  name: 'x64',
  type: 'target',
  dependencies,

  config() {
    return {
      'target.name': 'x64',
      'target.initramfs_dependencies': [],
      'target.linux_arch': 'x86',
      'target.deb_arch': 'amd64',
      'target.node_arch': 'x64',
      'target.node_gyp_arch': 'x64',
      'target.libTriplet': 'x86_64-linux-gnu',
      'target.configure_host': 'x86_64-linux-gnu',
      'target.build_prefix': `${buildPath('usr')}`,
      'glibc.ldDestPath': '/lib64/ld-linux-x86-64.so.2',
      'glibc.ldSourcePath': 'lib/x86_64-linux-gnu/ld-linux-x86-64.so.2',
      'target.toolchain_prefix': '', // This assumes that the x86_64 build machine.
      'openssl.configure_target': 'linux-x86_64',
      'linux.version': version,
      'linux.url': `https://cdn.kernel.org/pub/linux/kernel/v4.x/linux-${version}.tar.xz`,
      'linux.sha256': '32915a33bb0b993b779257748f89f31418992edba53acbe1160cb0f8ef3cb324',
      'linux.make_target': 'bzImage',
      'linux.bootfs': {
        '/EFI/boot/bootx64.efi': 'arch/x86/boot/bzImage'
      },
      'linux.rootfs': {}
    }
  },

  changed() {
    return dependencies.some(isRebuilt)
  },

  buildImage(imageFile) {
    buildFatImage(imageFile)
  }
}
