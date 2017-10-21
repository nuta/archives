const buildPlugin = require('../plugin_builder')

module.exports = (args, opts, logger) => {
  buildPlugin(args.path)
}
