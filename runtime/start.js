const fs = require('fs')
const { logger, Config, eprintln } = require('@makestack/runtime')

process.on('unhandledRejection', (reason, p) => {
  console.log('runtime: unhandled rejection:\n', reason, '\n\n', p)
  console.log('runtime: exiting...')
  process.exit(1)
})

process.on('message', (data) => {
  switch (data.type) {
    case 'initialize':
      logger.info(`initialize message: configs=${JSON.stringify(data.configs)}`)
      Config.update(data.configs)

      // Start the app.
      logger.info('staring the app')

      if (!fs.existsSync('./app.js')) {
        eprintln('app.js not found')
        return
      }

      require('./app')
      logger.info('started the app')
      break

    case 'configs':
      logger.info(`configs message: configs=${JSON.stringify(data.configs)}`)
      Config.update(data.configs)
      break

    default:
      logger.info('unknown ipc message: ', data)
  }
})

logger.info("waiting for `initialize' message from Supervisor...");
