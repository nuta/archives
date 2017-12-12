const fs = require('fs')
const path = require('path')
const { spawnSync } = require('child_process')
const { config, run, buildPath, isNewerDirContent } = require('../pkgbuilder').pkg

const ignore = [/^node_modules/]
const supervisorPath = path.resolve(__dirname, '../../supervisor')
const runtimePath = path.resolve(__dirname, '../../runtime')

module.exports = {
  name: 'supervisor',
  type: 'application',
  path: supervisorPath,
  ignore,

  changed() {
    return isNewerDirContent(supervisorPath, buildPath('supervisor'), ignore)
  },

  build() {
    let supervisorPackageJSON = JSON.parse(fs.readFileSync('./package.json'))
    supervisorPackageJSON.dependencies['@makestack/runtime'] = 'file:' + runtimePath
    fs.writeFileSync('package.json', JSON.stringify(supervisorPackageJSON))

    // Install dependencies to build.
    run(['yarn', '--ignore-scripts'])

    // Build Runtime.```
    run(['yarn', '--ignore-scripts'], {}, 'node_modules/@makestack/runtime')
    run(['npm', 'run', 'transpile'], {}, 'node_modules/@makestack/runtime')
    run(['npm', 'run', 'build-native'], {
      ARCH: config('target.node_gyp_arch'),
      AR: `${config('target.toolchain_prefix')}ar`,
      CC: `${config('target.toolchain_prefix')}gcc`,
      CXX: `${config('target.toolchain_prefix')}g++`,
      LINK: `${config('target.toolchain_prefix')}g++`
    }, 'node_modules/@makestack/runtime')

    // Build Supervisor after the runtime build since `prepare' hook of
    // runtime is not invoked if we simply run `yarn --production'.
    run(['yarn', 'build'])
  },

  rootfs() {
    const runtime = '/supervisor/node_modules/@makestack/runtime'
    return {
      '/supervisor/package.json': 'package.json',
      '/supervisor/supervisor': 'supervisor',
      '/supervisor/dist': 'dist',
      [`${runtime}/package.json`]: 'node_modules/@makestack/runtime/package.json',
      [`${runtime}/dist`]: 'node_modules/@makestack/runtime/dist',
      [`${runtime}/native`]: 'node_modules/@makestack/runtime/native'
    }
  }
}
