const path = require('path')
const logger = require('./logger')
const AppAPI = require('./api/app')
const SubProcessAPI = require('./api/subprocess')
const LoggingAPI = require('./api/logging')
const TimerAPI = require('./api/timer')
const StoreAPI = require('./api/store')
const EventAPI = require('./api/event')
const SerialAPI = require('./api/serial')
const plugin = require('./api/plugin')

let builtins = {
  Logging: new LoggingAPI(),
  Timer: new TimerAPI(),
  Store: new StoreAPI(),
  Event: new EventAPI(),
  App: new AppAPI(),
  SubProcess: new SubProcessAPI(),
  Serial: SerialAPI,
  plugin
}

if (process.env.MAKESTACK_DEVICE_TYPE) {
  const deviceType = process.env.MAKESTACK_DEVICE_TYPE
  const device = require(`./devices/${deviceType}`)
  Object.assign(builtins, device.initialize())
}

function start(appDir) {
  process.on('unhandledRejection', (reason, p) => {
    console.log('runtime: unhandled rejection:\n', reason, '\n\n', p)
    console.log('runtime: exiting...')
    process.exit(1)
  })

  Object.assign(global, builtins)

  process.on('message', (data) => {
    switch (data.type) {
      case 'initialize':
        logger.info(`initialize message: stores=${JSON.stringify(data.stores)}`)
        builtins.Store.update(data.stores)

        // Start the app.
        logger.info('staring the app')
        process.chdir(appDir)
        require(path.resolve(appDir, 'app'))
        logger.info('started the app')
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
  builtins,
  start
}
