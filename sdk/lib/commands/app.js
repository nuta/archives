let api = require('../api')

async function list(args, opts, logger) {
  for (const app of await api.getApps()) {
    logger.info(app)
  }
}

async function create(args, opts, logger) {
  await api.createApp(args.name, opts.api)
}

async function delete_(args, opts, logger) {
  await api.deleteApp(args.name)
}

module.exports = {
  list,
  create,
  delete_
}
