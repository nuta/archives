const drivelist = require('drivelist')
const chalk = require('chalk')

module.exports = (args, opts, logger) => {
  drivelist.list((error, drives) => {
    if(error)
    throw error

    for (let i = 0; i < drives.length; i++) {
      if (i > 0)
        console.log('')
      
      console.log(`Drive: ${chalk.bold.green(drives[i].device)}`)
      console.log(`  name: ${drives[i].displayName}`)
      console.log(`  description: ${drives[i].description}`)
      console.log(`  size: ${drives[i].size / 1024 / 1024}MiB`)
    }
  })
};