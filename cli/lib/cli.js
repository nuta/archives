
const program = require('caporal')

program
  .command('login', 'Get an access credential for an MakeStack Server account.')
  .action(require('./commands/login'))
  .command('mock create', 'Create a mock device.')
  .action(require('./commands/mock').create)
  .argument('name', 'The mock device name.')
  .command('mock run', 'Run a mock device.')
  .argument('name', 'The mock device name.')
  .action(require('./commands/mock').run)
  .command('list-drives', 'List available drives to install MakeStack OS/Linux.')
  .action(require('./commands/list_drives'))
  .command('install', 'Install MakeStack OS/Linux to the device.')
  .option('--name <name>', 'The device name.', null, null, true)
  .option('--type <type>', 'The device type.', /^raspberrypi3$/, null, true)
  .option('--os <os>', 'The OS image.', /^linux$/, null, true)
  .option('--drive <drive>', "The drive. Use `list-drives' command.", null, null, true)
  .option('--adapter <adapter>', 'The network adapter.', /^ethernet$/, null, true)
  .option('--ignore-duplication', 'Ignore duplicated device name.', null, false)
  .action(require('./commands/install'))

function run(argv) {
  program.parse(argv)
}

module.exports = { run }
