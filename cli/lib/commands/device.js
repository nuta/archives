let api = require('../api')

async function list(args, opts, logger) {
  for (const device of await api.getDevices()) {
    logger.info(device)
  }
}

async function delete_(args, opts, logger) {
  await api.deleteDevice(args.name)
}

module.exports = {
  list,
  delete_
}
