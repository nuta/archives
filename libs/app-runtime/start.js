const { LoggingAPI, TimerAPI, StoreAPI, EventAPI, GPIOAPI, I2CAPI } = require('app-runtime')

global.Logging = new LoggingAPI()
global.Timer = new TimerAPI()
global.Store = new StoreAPI()
global.Event = new EventAPI()
global.GPIO = new GPIOAPI()
global.I2C = new I2CAPI()

process.on('message', (data) => {
  console.log('***', data)

  switch (data.type) {
    case 'initialize':
      console.log(data.stores)
      Store.update(data.stores)
      require('./main')
      break
  }
})

console.log('*** starting app')
