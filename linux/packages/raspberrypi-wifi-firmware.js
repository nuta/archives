const { applyPatch } = require('../pkgbuilder').pkg

const version = '1.20170811'

module.exports = {
  name: 'raspberrypi-wifi-firmware',
  type: 'library',
  version,
  url: `aa/archive/master.zip`,
  sha256: 'f45cc567ea4d6370a4e43e8fe28f3bcdb303d65bb8a17cffa82f9fe492e6e7e9',

  build() {
    applyPatch('debian/patches/brcmfmac43430-sdio-txt.patch')
  },

  rootfs: {
    '/lib/firmware/brcm/brcmfmac43430-sdio.bin': 'brcm/brcmfmac43430-sdio.bin',
    '/lib/firmware/brcm/brcmfmac43430-sdio.txt': 'brcm/brcmfmac43430-sdio.txt'
  }
}
