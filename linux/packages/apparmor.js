const { config, assetPath } = require('../pkgbuilder').pkg

const version = '2.10.95-0ubuntu2'

module.exports = {
  name: 'apparmor',
  type: 'application',
  version,

  url() {
    return `${config('target.ubuntu_pkg_url')}/pool/main/a/apparmor/apparmor_${version}_${config('target.deb_arch')}.deb`
  },

  sha256() {
    switch (config('target.deb_arch')) {
      case 'amd64': return 'e5a26afe7f04b5d8b6292a5c6fbb5482797e410bda4ab8720cba528f3ba1fb3f'
      case 'armhf': return 'e3b8c1d7d88e34e92af91309f860dae8f8e454211cf39217ced609afc47cf87b'
      default: throw new Error(`unknown target.deb_arch: \`${config('deb_arch')}'`)
    }
  },

  changed() {
    return false
  },

  build() {
  },

  rootfs() {
    return {
      '/bin/apparmor_parser': `sbin/apparmor_parser`,
      '/etc/apparmor.d/bin.node': assetPath('apparmor', 'bin.node')
    }
  }
}
