const os = require('os')
const path = require('path')

module.exports = async (args, opts, logger) => {
  if (opts.adapter === 'http' && !opts.server) {
    throw new Error('--server is missing')
  }

  process.env.RUNTIME_MODULE = path.resolve(__dirname, '../../../runtime')
  process.env.MAKESTACK_DEVICE_TYPE = 'sdk'
  const Supervisor = require('../../../supervisor')
  const supervisor = new Supervisor({
    appDir: path.resolve(os.homedir(), '.makestack/app'),
    adapter: {
      name: opts.adapter,
      url: opts.server
    },
    osType: 'sdk',
    deviceType: 'sdk',
    deviceId: opts.deviceId,
    deviceSecret: opts.deviceSecret,
    debugMode: true,
    osVersion: null,
    heartbeatInterval: opts.heartbeatInterval
  })

  supervisor.start()
}
