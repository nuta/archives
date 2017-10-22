const os = require('os')
const logger = require('./logger')
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

module.exports = (mainModulePath) => {

  global.Logging = new LoggingAPI()
  global.Timer = new TimerAPI()
  global.Store = new StoreAPI()
  global.Event = new EventAPI()
  global.GPIO = new GPIOAPI()
  global.I2C = new I2CAPI()

  process.on('message', (data) => {
    switch (data.type) {
      case 'initialize':
      logger.info(`initialize message: stores=${JSON.stringify(data.stores)}`)
      Store.update(data.stores)
        require(mainModulePath)
        break

      case 'stores':
        logger.info(`stores message: stores=${JSON.stringify(data.stores)}`)
        Store.update(data.stores)
        break

      default:
        logger.info('unknown ipc message: ', data)
    }
  })

  logger.info("waiting for `initialize' message from Supervisor...")
}
