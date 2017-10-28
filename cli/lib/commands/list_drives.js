const chalk = require('chalk')
const { getAvailableDrives } = require('../drive')

module.exports = async(args, opts, logger) => {
  const drives = await getAvailableDrives()
  for (const [i, drive] of drives.entries()) {
    if (i > 0) { console.log('') }

    console.log(`Drive: ${chalk.bold.green(drive.device)}`)
    console.log(`  name: ${drive.displayName}`)
    console.log(`  description: ${drive.description}`)
    console.log(`  size: ${drive.size / 1024 / 1024}MiB`)
  }
}
