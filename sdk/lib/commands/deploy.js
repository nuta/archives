const { deployAppDir } = require('../deploy')

module.exports = async (args, opts) => {
  await deployAppDir(opts.appDir)
}
