const { assetPath } = require('../pkgbuilder').pkg

const version = '1.20170811'

module.exports = {
  name: 'raspberrypi-wifi-firmware',
  type: 'library',
  version,
  url: `https://github.com/RPi-Distro/firmware-nonfree/archive/master.zip`,
  sha256: 'f782db09102b6aca90191ca6996cbca5fb91bafd500ee40c1de8e951720ac06f',

  rootfs: {
    '/lib/firmware/brcm/brcmfmac43430-sdio.bin': 'brcm80211/brcm/brcmfmac43430-sdio.bin',
    '/lib/firmware/brcm/brcmfmac43430-sdio.txt': 'brcm80211/brcm/brcmfmac43430-sdio.txt'
  }
}
