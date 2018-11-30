const { assetPath, defineUbuntuPackage } = require('../pkgbuilder').pkg

module.exports = defineUbuntuPackage('apparmor', {
  name: 'apparmor',
  type: 'application',
  rootfs: {
    '/bin/apparmor_parser': `sbin/apparmor_parser`,
    '/etc/apparmor.d/bin.node': assetPath('apparmor', 'bin.node')
  }
})
