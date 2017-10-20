const { config, assetPath } = require('../pkgbuilder').pkg

const version = '2.10.95-0ubuntu2'

module.exports = {
  name: 'apparmor',
  type: 'application',
  version,

  url() {
    return `${config('target.ubuntu_pkg_url')}/pool/main/a/apparmor/apparmor_${version}_${config('target.deb_arch')}.deb`
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
