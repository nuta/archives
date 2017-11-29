const { deployAppDir } = require('../deploy')

export async function main(args, opts, logger) {
  await deployAppDir(opts.appDir)
}
