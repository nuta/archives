const fs = require('fs')
const ioctl = require('../ioctl')

const I2C_SLAVE = 0x0703

module.exports = class {
  constructor({ address }) {
    this.address = address

    if (!fs.existsSync('/dev/i2c-1')) {
      console.log('i2c device not found, disabling I2C API')
      this.fd = fs.openSync('/dev/null', 'r')
      return
    }

    this.fd = fs.openSync('/dev/i2c-1', 'rs+')
  }

  reset() {
    fs.closeSync(this.fd)
  }

  read(length) {
    this.selectSlaveAddress(this.address)
    let buffer = Buffer.alloc(length)
    fs.readSync(this.fd, buffer, 0, length)
    return buffer
  }

  write(data) {
    this.selectSlaveAddress(this.address)
    fs.writeSync(this.fd, Buffer.from(data))
  }

  selectSlaveAddress(address) {
    if (ioctl(this.fd, I2C_SLAVE, address) !== 0) {
      throw new Error('failed to set I2C_SLAVE')
    }
  }
}
