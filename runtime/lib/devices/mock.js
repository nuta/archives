const GPIO = require('../mock/gpio')
const I2C = require('../mock/i2c')

function initialize() {
  return { I2C, GPIO }
}

module.exports = { initialize }
