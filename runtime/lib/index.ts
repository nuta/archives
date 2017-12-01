import * as path from 'path';
import { logger } from './logger';
import { AppAPI } from './api/app';
import { SubProcessAPI } from './api/subprocess';
import { println, error } from './api/logging';
import { TimerAPI } from './api/timer';
import { StoreAPI } from './api/store';
import { publish } from './api/event';
import { SerialAPI } from './api/serial';

let builtins = {
  Timer: new TimerAPI(),
  Store: new StoreAPI(),
  App: new AppAPI(),
  SubProcess: new SubProcessAPI(),
  Serial: SerialAPI,
  println,
  error,
  publish
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
