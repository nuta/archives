const GPIO = require('../linux/gpio')
const I2C = require('../linux/i2c')

function initialize() {
  return { I2C, GPIO }
}

module.exports = { initialize }
