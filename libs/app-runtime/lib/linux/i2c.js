const fs = require('fs')
const { ioctl } = require('../ioctl')

const I2C_SLAVE = 0x0703

module.exports = class {
  constructor() {
    if (!fs.existsSync('/dev/i2c-1')) {
      console.log('i2c device not found, disabling I2C API')
      this.fd = fs.openSync('/dev/null')
      return
    }

    this.fd = fs.openSync('/dev/i2c-1', 'r+')
  }

  reset() {
  }

  read(address, length) {
    this.selectSlaveAddress(address)
    let buffer = Buffer.alloc(length)
    fs.readSync(this.fd, buffer, 0, length)
    return buffer
  }

  write(address, data) {
    this.selectSlaveAddress(address)
    fs.writeSync(this.fd, Buffer.from(data))
  }

  selectSlaveAddress(address) {
    if (ioctl(this.fd, I2C_SLAVE, address) !== 0) {
      throw new Error('failed to open I2C bus')
    }
  }
}
