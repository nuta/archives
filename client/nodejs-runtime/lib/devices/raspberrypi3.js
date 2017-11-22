const GPIO = require('../linux/gpio')
const I2C = require('../linux/i2c')
const SPI = require('../linux/spi')

function initialize() {
  return { I2C, GPIO, SPI }
}

module.exports = { initialize }
