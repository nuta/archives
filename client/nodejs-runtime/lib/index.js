const logger = require('./logger')
const AppAPI = require('./api')
const LoggingAPI = require('./logging')
const TimerAPI = require('./timer')
const StoreAPI = require('./store')
const EventAPI = require('./event')
const plugin = require('./plugin')

let builtins = {
  Logging: new LoggingAPI(),
  Timer: new TimerAPI(),
  Store: new StoreAPI(),
  Event: new EventAPI(),
  App: new AppAPI(),
  plugin
}

const mock = require(`./devices/mock`)
Object.assign(builtins, mock.initialize())

if (process.env.DEVICE_TYPE) {
  const deviceType = process.env.DEVICE_TYPE
  const device = require(`./devices/${deviceType}`)
  Object.assign(builtins, device.initialize())
}

function start(mainModulePath) {
  Object.assign(global, builtins)

  process.on('message', (data) => {
    switch (data.type) {
      case 'initialize':
        logger.info(`initialize message: stores=${JSON.stringify(data.stores)}`)
        builtins.Store.update(data.stores)
        require(mainModulePath)
        break

      case 'stores':
        logger.info(`stores message: stores=${JSON.stringify(data.stores)}`)
        builtins.Store.update(data.stores)
        break

      default:
        logger.info('unknown ipc message: ', data)
    }
  })

  logger.info("waiting for `initialize' message from Supervisor...")
}

module.exports = {
  Driver: require('./driver'),
  builtins,
  start
}
