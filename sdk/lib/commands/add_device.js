const api = require('../api')
const { loadAppJSON } = require('../appdir')

async function addDevice(args, opts, logger) {
  const appName = loadAppJSON(opts.appDir).name

  await api.updateDevice(args.deviceName, {
    app: appName
  })
}

module.exports = addDevice
