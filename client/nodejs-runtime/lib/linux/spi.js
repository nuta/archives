const fs = require('fs')
const GPIO = require('./gpio')
const spi = require(`../../native/${process.arch}/spi.node`)

const SPI_CPHA = 0x01
const SPI_CPOL = 0x02
const SPI_MODES = {
  MODE0: 0x00,
  MODE1: SPI_CPHA,
  MODE2: SPI_CPOL,
  MODE3: SPI_CPHA | SPI_CPOL
}

class SPIAPI {
  constructor({ path, ss, speed, order, mode, bits }) {
    this.fd = fs.openSync(path, 'rs+')
    this.ss = new GPIO({ pin: ss, mode: GPIO.OUTPUT })
    this.ss.write(true)
    this.configure(mode, bits, speed, order)
  }

  configure(mode, bits, speed, order) {
    this.mode = mode || 'MODE0'
    this.bits = bits || 8
    this.speed = speed || 500000
    this.order = order || 'MSBFIRST'

    const modeNumber = SPI_MODES[this.mode]
    if (typeof modeNumber !== 'number') {
        throw new Error('invalid spi mode')
    }

    spi.configure(
      this.fd,
      modeNumber,
      this.bits,
      this.speed,
      (this.order == 'LSBFIRST') ? 1 : 0
    )
  }

  transfer(tx) {
    const rx = Buffer.alloc(tx.length)
    this.ss.write(false)
    try {
      spi.transfer(this.fd, this.speed, Buffer.from(tx), rx)
    } finally {
      this.ss.write(true)
    }
    return rx
  }
}

module.exports = SPIAPI
