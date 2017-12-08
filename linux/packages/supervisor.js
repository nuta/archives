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

  check() {
    if (spawnSync('which', ['node-gyp']).status !== 0) {
      throw new Error('Install node-gyp and nan first!')
    }
  },

  changed() {
    const isChanged = isNewerDirContent(supervisorPath, buildPath('supervisor'), ignore)
    if (isChanged) {
      // TODO: do this in pkgbuilder
      run([ 'rm', '-r', buildPath('supervisor') ])
      run([ 'cp', '-r', supervisorPath, buildPath('supervisor') ])
    }

    return isChanged
  },

  build() {
    let packageJSON = JSON.parse(fs.readFileSync('package.json'))
    packageJSON.dependencies['@makestack/runtime'] = 'file:' + runtimePath
    fs.writeFileSync('package.json', JSON.stringify(packageJSON))

    run(['yarn'])
    run(['npm', 'run', 'transpile'], {}, 'node_modules/@makestack/runtime')
    run(['npm', 'run', 'build-native'], {
      ARCH: config('target.node_gyp_arch'),
      AR: `${config('target.toolchain_prefix')}ar`,
      CC: `${config('target.toolchain_prefix')}gcc`,
      CXX: `${config('target.toolchain_prefix')}g++`,
      LINK: `${config('target.toolchain_prefix')}g++`
    }, 'node_modules/@makestack/runtime')

    run(['rm', '-rf', 'node_modules/@makestack/runtime/node_modules'])
  },

  rootfs() {
    return {
      '/supervisor/package.json': 'package.json',
      '/supervisor/supervisor': 'supervisor',
      '/supervisor/dist': 'dist',
      '/supervisor/node_modules/@makestack/runtime': 'node_modules/@makestack/runtime'
    }
  }
}
