const fs = require('fs')
const path = require('path')
const { config, mkdirp, run, buildPath, rootfsPath, isNewerDirContent } = require('../pkgbuilder').pkg

const ignore = [/^node_modules/]
const supervisorPath = path.resolve(__dirname, '../../libs/supervisor')
const appRuntimePath = path.resolve(__dirname, '../../libs/app-runtime')

module.exports = {
  name: 'supervisor',
  type: 'application',
  path: supervisorPath,
  ignore,

  changed() {
    return isNewerDirContent(supervisorPath, buildPath('supervisor'), ignore)
  },

  build() {
    let packageJSON = JSON.parse(fs.readFileSync('package.json'))
    packageJSON.dependencies['app-runtime'] = 'file:' + appRuntimePath
    fs.writeFileSync('package.json', JSON.stringify(packageJSON))

    const nodeGyp = buildPath('node-gyp/node_modules/.bin/node-gyp')
    if (!fs.existsSync(nodeGyp)) {
      mkdirp(path.dirname(nodeGyp))
      run(['npm', 'i', 'node-gyp'], {}, path.dirname(nodeGyp))
    }

    run(['yarn'])
    run(['rm', '-rf', 'node_modules/app-runtime/build'])
    run([nodeGyp, 'rebuild', '--arch', config('target.node_gyp_arch')], {
      AR: `${config('target.toolchain_prefix')}ar`,
      CC: `${config('target.toolchain_prefix')}gcc`,
      CXX: `${config('target.toolchain_prefix')}g++`,
      LINK: `${config('target.toolchain_prefix')}g++`
    }, 'node_modules/app-runtime')
  },

  rootfs() {
    return {
      '/supervisor': `.`
    }
  }
}
