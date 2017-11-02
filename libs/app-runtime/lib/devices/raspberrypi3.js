const GPIO = require('../linux/gpio')
const I2C = require('../linux/i2c')

function initialize() {
  API.register('I2C', I2C)
  API.register('GPIO', GPIO)
}

module.exports = initialize
