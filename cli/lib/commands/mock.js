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
  let mock = config.mocks[args.name];
  (new Supervisor(config.server.url, mock.device_id)).start()
}

module.exports = { create, run }
