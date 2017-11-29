const path = require('path')
const chalk = require('chalk')
const install = require('../install')

function progress(stage, state) {
  switch (stage) {
    case 'look-for-drive':
      console.info(chalk.bold.blue('==> (1/5) Looking for the drive'))
      break
    case 'register':
      console.info(chalk.bold.blue('==> (2/5) Registering the device'))
      break
    case 'download':
      console.info(chalk.bold.blue('==> (3/5) Downloading the disk image'))
      break
    case 'config':
      console.info(chalk.bold.blue('==> (4/5) Writing config'))
      break
    case 'flash':
      console.info(chalk.bold.blue('==> (5/5) Flashing'))
      break
    case 'flashing':
      const message = { write: 'Writing', check: 'Verifying' }[state.type]
      console.info(`${message}...(${state.percentage}%)`)
      break
  }
}

export async function main(args, opts, logger) {
  await install({
    deviceName: opts.name,
    deviceType: opts.type,
    osType: opts.os,
    adapter: opts.adapter,
    drive: opts.drive,
    ignoreDuplication: opts.ignoreDuplication,
    flashCommand: [process.argv0, path.resolve(__dirname, '../../bin/makestack')]
  }, progress)

  console.info(chalk.bold.green('Done!'))
}
