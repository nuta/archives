const { applyPatch } = require('../pkgbuilder').pkg

const version = '1.20170811'

module.exports = {
  name: 'raspberrypi-wifi-firmware',
  type: 'library',
  version,
  url: 'https://github.com/RPi-Distro/firmware-nonfree/archive/927fa8ebdf5bcfb90944465b40ec4981e01d6015.zip',
  sha256: 'f521a945eb8e555aaa185554b66664e19f037be334d061374aad47f82f2166d1',

  rootfs: {
    '/lib/firmware/brcm/brcmfmac43430-sdio.bin': 'brcm/brcmfmac43430-sdio.bin',
    '/lib/firmware/brcm/brcmfmac43430-sdio.txt': 'brcm/brcmfmac43430-sdio.txt'
  }
}
