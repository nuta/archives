let os = require('os')
let path = require('path')
let Supervisor = require('supervisor')
let api = require('../api')
let config = require('../config')

function create(args, opts, logger) {
  let deviceName = args.name
  api.registerDevice(deviceName, 'mock', null).then(r => {
    Object.assign(config.mocks, { deviceName: r.json })
  }).catch(e => {
    logger.error('failed to create a mock device', e)
  })
}

function run(args, opts, logger) {
  let mock = config.mocks[args.name]
  const osVersion = 'a' // A version defined in server/config/makestack.yml

  const supervisor = new Supervisor({
    appDir: path.resolve(os.homedir(), '.makestack/mock-app'),
    adapter: {
      name: 'http',
      url: config.server.url
    },
    deviceType: 'mock',
    deviceId: mock.device_id,
    debugMode: true,
    osVersion
  })

  supervisor.start()
}

module.exports = { create, run }
