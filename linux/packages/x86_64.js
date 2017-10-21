const { isRebuilt } = require('../pkgbuilder').pkg

const version = '4.9.53'
const dependencies = ['linux', 'bootfs-files']

module.exports = {
  name: 'x86_64',
  type: 'target',
  dependencies,

  config() {
    return {
      'target.name': 'x86_64',
      'target.linux_arch': 'x86',
      'target.deb_arch': 'amd64',
      'target.node_arch': 'x64',
      'target.node_gyp_arch': 'x64',
      'target.libTriplet': 'x86_64-linux-gnu',
      'target.ubuntu_pkg_url': 'http://us.archive.ubuntu.com/ubuntu',
      'glibc.ldDestPath': '/lib64/ld-linux-x86-64.so.2',
      'glibc.ldSourcePath': 'lib/x86_64-linux-gnu/ld-linux-x86-64.so.2',
      'target.toolchain_prefix': '', // This assumes that the x86_64 build machine.
      'linux.version': version,
      'linux.url': `https://cdn.kernel.org/pub/linux/kernel/v4.x/linux-${version}.tar.xz`,
      'linux.sha256': '32915a33bb0b993b779257748f89f31418992edba53acbe1160cb0f8ef3cb324',
      'linux.make_target': 'bzImage',
      'linux.bootfs': {
        '/vmlinuz': 'arch/x86/boot/bzImage'
      },
      'linux.rootfs': {}
    }
  },

  changed() {
    return dependencies.some(isRebuilt)
  },

  buildImage(imageFile) {
    // TODO
  }
}
