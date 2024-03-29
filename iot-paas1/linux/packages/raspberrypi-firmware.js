const { assetPath } = require('../pkgbuilder').pkg

const version = '1.20170811'

module.exports = {
  name: 'raspberrypi-firmware',
  type: 'library',
  version,
  url: `https://github.com/raspberrypi/firmware/archive/${version}.tar.gz`,
  sha256: 'a25f6281d64732892a2e838cc2346f1a88505b5c77a57a6540755362ea64043a',

  bootfs() {
    const srcFiles = [
      'LICENCE.broadcom',
      'COPYING.linux',
      'bootcode.bin',
      'start.elf',
      'start_x.elf',
      'start_cd.elf',
      'start_db.elf',
      'fixup.dat',
      'fixup_x.dat',
      'fixup_cd.dat',
      'fixup_db.dat',
      'bcm2708-rpi-0-w.dtb',
      'bcm2709-rpi-2-b.dtb',
      'bcm2710-rpi-3-b.dtb',
      'bcm2708-rpi-b.dtb',
      'bcm2708-rpi-b-plus.dtb',
      'bcm2708-rpi-cm.dtb',
      'bcm2710-rpi-cm3.dtb',
      'overlays/lirc-rpi.dtbo',
      'overlays/rpi-cirrus-wm5102.dtbo',
      'overlays/i2c-pwm-pca9685a.dtbo',
      'overlays/rpi-sense.dtbo',
      'overlays/spi0-cs.dtbo',
      'overlays/spi-gpio35-39.dtbo',
      'overlays/pitft35-resistive.dtbo',
      'overlays/ads1015.dtbo',
      'overlays/i2c-rtc-gpio.dtbo',
      'overlays/wittypi.dtbo',
      'overlays/rpi-dac.dtbo',
      'overlays/sdhost.dtbo',
      'overlays/mcp3008.dtbo',
      'overlays/sdio-1bit.dtbo',
      'overlays/enc28j60-spi2.dtbo',
      'overlays/hy28a.dtbo',
      'overlays/rpi-backlight.dtbo',
      'overlays/dwc-otg.dtbo',
      'overlays/hifiberry-amp.dtbo',
      'overlays/fe-pi-audio.dtbo',
      'overlays/rpi-proto.dtbo',
      'overlays/uart1.dtbo',
      'overlays/at86rf233.dtbo',
      'overlays/justboom-dac.dtbo',
      'overlays/rpi-tv.dtbo',
      'overlays/pitft28-resistive.dtbo',
      'overlays/enc28j60.dtbo',
      'overlays/audioinjector-wm8731-audio.dtbo',
      'overlays/spi1-3cs.dtbo',
      'overlays/dionaudio-loco-v2.dtbo',
      'overlays/justboom-digi.dtbo',
      'overlays/pitft28-capacitive.dtbo',
      'overlays/spi2-1cs.dtbo',
      'overlays/iqaudio-dacplus.dtbo',
      'overlays/rotary-encoder.dtbo',
      'overlays/spi2-2cs.dtbo',
      'overlays/mcp23017.dtbo',
      'overlays/midi-uart1.dtbo',
      'overlays/spi2-3cs.dtbo',
      'overlays/README',
      'overlays/spi1-2cs.dtbo',
      'overlays/dionaudio-loco.dtbo',
      'overlays/googlevoicehat-soundcard.dtbo',
      'overlays/mmc.dtbo',
      'overlays/gpio-shutdown.dtbo',
      'overlays/gpio-poweroff.dtbo',
      'overlays/pi3-miniuart-bt.dtbo',
      'overlays/adau1977-adc.dtbo',
      'overlays/pitft22.dtbo',
      'overlays/bmp085_i2c-sensor.dtbo',
      'overlays/i2c-bcm2708.dtbo',
      'overlays/vga666.dtbo',
      'overlays/sc16is750-i2c.dtbo',
      'overlays/pwm-2chan.dtbo',
      'overlays/vc4-kms-v3d.dtbo',
      'overlays/i2c-mux.dtbo',
      'overlays/mcp2515-can0.dtbo',
      'overlays/gpio-ir.dtbo',
      'overlays/sdio.dtbo',
      'overlays/raspidac3.dtbo',
      'overlays/hifiberry-digi-pro.dtbo',
      'overlays/mz61581.dtbo',
      'overlays/allo-boss-dac-pcm512x-audio.dtbo',
      'overlays/mcp23s17.dtbo',
      'overlays/iqaudio-digi-wm8804-audio.dtbo',
      'overlays/pwm.dtbo',
      'overlays/goodix.dtbo',
      'overlays/pps-gpio.dtbo',
      'overlays/pi3-disable-bt.dtbo',
      'overlays/papirus.dtbo',
      'overlays/piscreen2r.dtbo',
      'overlays/rpi-display.dtbo',
      'overlays/spi1-1cs.dtbo',
      'overlays/audioinjector-addons.dtbo',
      'overlays/spi0-hw-cs.dtbo',
      'overlays/i2s-gpio28-31.dtbo',
      'overlays/ads1115.dtbo',
      'overlays/hifiberry-dacplus.dtbo',
      'overlays/pisound.dtbo',
      'overlays/adau7002-simple.dtbo',
      'overlays/i2c-gpio.dtbo',
      'overlays/rpi-ft5406.dtbo',
      'overlays/mcp2515-can1.dtbo',
      'overlays/piscreen.dtbo',
      'overlays/sc16is752-spi1.dtbo',
      'overlays/w1-gpio.dtbo',
      'overlays/pi3-act-led.dtbo',
      'overlays/rra-digidac1-wm8741-audio.dtbo',
      'overlays/ads7846.dtbo',
      'overlays/dpi24.dtbo',
      'overlays/dht11.dtbo',
      'overlays/iqaudio-dac.dtbo',
      'overlays/dwc2.dtbo',
      'overlays/i2c0-bcm2708.dtbo',
      'overlays/spi-rtc.dtbo',
      'overlays/pi3-disable-wifi.dtbo',
      'overlays/allo-piano-dac-plus-pcm512x-audio.dtbo',
      'overlays/i2c-rtc.dtbo',
      'overlays/hifiberry-dac.dtbo',
      'overlays/smi.dtbo',
      'overlays/midi-uart0.dtbo',
      'overlays/tinylcd35.dtbo',
      'overlays/i2c-sensor.dtbo',
      'overlays/akkordion-iqdacplus.dtbo',
      'overlays/allo-piano-dac-pcm512x-audio.dtbo',
      'overlays/hifiberry-digi.dtbo',
      'overlays/audremap.dtbo',
      'overlays/dpi18.dtbo',
      'overlays/i2c1-bcm2708.dtbo',
      'overlays/w1-gpio-pullup.dtbo',
      'overlays/vc4-fkms-v3d.dtbo',
      'overlays/hy28b.dtbo',
      'overlays/smi-dev.dtbo',
      'overlays/sdtweak.dtbo',
      'overlays/smi-nand.dtbo',
      'overlays/allo-digione.dtbo',
      'overlays/mpu6050.dtbo',
      'overlays/qca7000.dtbo'
    ]

    const files = {}
    for (const relpath of srcFiles) {
      files[`/${relpath}`] = `boot/${relpath}`
    }

    return files
  }
}
