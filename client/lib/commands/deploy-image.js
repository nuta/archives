const fs = require('fs')
const api = require('../api')

module.exports = async (args, opts) => {
  await api.deploy(opts.app, fs.readFileSync(args.image))
}
