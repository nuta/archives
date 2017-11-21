let os = require('os')
let path = require('path')
let api = require('../api')
let { loadMocks, updateMocks } = require('../config')

function create(args, opts, logger) {
  let deviceName = args.name
  api.registerDevice(deviceName, 'mock', null).then(device => {
    updateMocks({ [deviceName]: device })
  }).catch(e => {
    logger.error('failed to create a mock device', e)
  })
}

function run(args, opts, logger) {
  let mock = loadMocks()[args.name]
  const osVersion = 'a' // A version defined in server/config/makestack.yml

  process.env.RUNTIME_MODULE = path.resolve(__dirname, '../../nodejs-runtime')
  process.env.MAKESTACK_DEVICE_TYPE = mock.device_type // FIXME: used by nodejs-runtime
  const Supervisor = require('../../supervisor')
  const supervisor = new Supervisor({
    appDir: path.resolve(os.homedir(), '.makestack/mock-app'),
    adapter: {
      name: opts.adapter,
      url: api.serverURL
    },
    osType: 'mock',
    deviceType: mock.device_type,
    deviceId: mock.device_id,
    deviceSecret: mock.device_secret,
    debugMode: true,
    osVersion,
    heartbeatInterval: 2
  })

  supervisor.start()
}

module.exports = { create, run }
