const { LinuxGPIOAPI } = require('../linux/gpio')
const { LinuxI2CAPI } = require('../linux/i2c')
const { LinuxSPIAPI } = require('../linux/spi')

class GPIO extends LinuxGPIOAPI {
}

const I2C_FILE = '/dev/i2c-1'
class I2C extends LinuxI2CAPI {
  get path() {
    return I2C_FILE
  }
}

const SPI_BUS = '0'
class SPI extends LinuxSPIAPI {
  get bus() {
    return SPI_BUS
  }
}

function initialize() {
  return { GPIO, I2C, SPI }
}

module.exports = { initialize }
