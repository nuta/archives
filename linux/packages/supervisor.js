const fs = require('fs')
const path = require('path')
const { spawnSync } = require('child_process')
const { config, run, buildPath, isNewerDirContent } = require('../pkgbuilder').pkg

const ignore = [/^node_modules/]
const supervisorPath = path.resolve(__dirname, '../../client/supervisor')
const appRuntimePath = path.resolve(__dirname, '../../client/nodejs-runtime')

module.exports = {
  name: 'supervisor',
  type: 'application',
  path: supervisorPath,
  ignore,

  check() {
    if (spawnSync('which', ['node-gyp']).status !== 0) {
      throw new Error('Install node-gyp first!')
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
    packageJSON.dependencies['nodejs-runtime'] = 'file:' + appRuntimePath
    fs.writeFileSync('package.json', JSON.stringify(packageJSON))

    run(['rm', '-rf', 'node_modules'])
    run(['yarn', '--production'])
    run(['npm', 'run', 'build'], {
      ARCH: config('target.node_gyp_arch'),
      AR: `${config('target.toolchain_prefix')}ar`,
      CC: `${config('target.toolchain_prefix')}gcc`,
      CXX: `${config('target.toolchain_prefix')}g++`,
      LINK: `${config('target.toolchain_prefix')}g++`
    }, 'node_modules/nodejs-runtime')
  },

  rootfs() {
    return {
      '/supervisor': `.`
    }
  }
}
