
const program = require('caporal')

program
  .command('login', 'Get an access credential for an MakeStack Server account.')
  .action(require('./commands/login'))

program
  .command('app list', 'List apps.')
  .action(require('./commands/app').list)

program
  .command('app create', 'Create an app.')
  .argument('name', 'The app name.')
  .option('--api <api>', 'The API.', /^nodejs$/, null, true)
  .action(require('./commands/app').create)

program
  .command('app delete', 'Delete an app.')
  .argument('name', 'The app name.')
  .action(require('./commands/app').delete_)

program
  .command('device list', 'List devices.')
  .action(require('./commands/device').list)

program
  .command('device delete', 'Delete an device.')
  .argument('name', 'The device name.')
  .action(require('./commands/device').delete_)

program
  .command('deploy', 'Deploy the app.')
  .option('--app-dir <app-dir>', 'The app directory.', null, process.cwd())
  .action(require('./commands/deploy'))

program
  .command('deploy-image', 'Deploy the app with a prebuilt image.')
  .option('--app <app>', 'The app name.', null, process.cwd())
  .argument('image', 'The prebuilt image.')
  .action(require('./commands/deploy-image'))

program
  .command('add-device', 'Add an device to the app.')
  .argument('name', 'The device name.')
  .option('--app-dir <app-dir>', 'The app directory.', null, process.cwd())
  .action(require('./commands/add_device'))

program
  .command('mock create', 'Create a mock device.')
  .argument('name', 'The mock device name.')
  .option('--device-type <device-type>', 'The device type.', /^raspberrypi3|mock$/, 'mock')
  .action(require('./commands/mock').create)

program
  .command('mock run', 'Run a mock device.')
  .argument('name', 'The mock device name.')
  .option('--adapter <adapter>', 'The network adapter.', /^http|sakuraio$/, 'http')
  .action(require('./commands/mock').run)

program
  .command('repl', 'A Read-Eval-Print-Loop (REPL) console.')
  .argument('name', 'The device name.')
  .action(require('./commands/repl'))

program
  .command('list-drives', 'List available drives to install MakeStack OS/Linux.')
  .action(require('./commands/list_drives'))

program
  .command('install', 'Install MakeStack OS/Linux to the device.')
  .option('--name <name>', 'The device name.', null, null, true)
  .option('--type <type>', 'The device type.', /^raspberrypi3|mock$/, null, true)
  .option('--os <os>', 'The OS image.', /^linux$/, null, 'linux')
  .option('--drive <drive>', "The drive. Use `list-drives' command.", null, null, true)
  .option('--adapter <adapter>', 'The network adapter.', /^http|sakuraio$/, 'http')
  .option('--ignore-duplication', 'Ignore duplicated device name.', null, false)
  .action(require('./commands/install'))

program
  .command('build-plugin', 'Build a plugin using Docker.')
  .argument('path', 'The plugin directory.')
  .argument('dest', 'The destination.')
  .action(require('./commands/build_plugin'))

program
  .command('red', 'Program the app with Node-RED.')
  .option('--port <port>', 'The port.', null, 31313)
  .option('--app-dir <app-dir>', 'The app directory.', null, process.cwd())
  .option('--dev', 'Restart Node-RED automatically for Node-RED node development.', null, false)
  .action(require('./commands/red'))

function run(argv) {
  program.parse(argv)
}

module.exports = { run }
