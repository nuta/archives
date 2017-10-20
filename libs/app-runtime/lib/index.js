const os = require('os')
const LoggingAPI = require('./logging')
const TimerAPI = require('./timer')
const StoreAPI = require('./store')
const EventAPI = require('./event')

let GPIOAPI, I2CAPI
switch (os.type()) {
  case 'Linux':
    GPIOAPI = require('./linux/gpio')
    I2CAPI = require('./linux/i2c')
    break
  default:
    GPIOAPI = require('./mock/gpio')
    I2CAPI = require('./mock/i2c')
    break
}

module.exports = {
  LoggingAPI,
  TimerAPI,
  StoreAPI,
  EventAPI,
  GPIOAPI,
  I2CAPI
}
