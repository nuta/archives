const path = require('path')
const {
  run, buildPath, isNewerDirContent, modifyJsonFile
} = require('../pkgbuilder').pkg

const ignore = [/^node_modules/]
const supervisorPath = path.resolve(__dirname, '../../supervisor')

module.exports = {
  name: 'supervisor',
  type: 'application',
  path: supervisorPath,
  ignore,
  dependencies: ['npm-packages'],

  changed() {
    return isNewerDirContent(supervisorPath, buildPath('supervisor'), ignore)
  },

  build() {
    modifyJsonFile('package.json', {
      dependencies: {
        'makestack': buildPath('npm-packages/node_modules/makestack')
      }
    })

    run(['yarn', '--no-progress', '--ignore-scripts'])
    run(['yarn', 'run', 'tsc'])
    run(['yarn', '--no-progress', '--ignore-scripts', '--production'])
  },

  rootfs() {
    return {
      '/supervisor/package.json': 'package.json',
      '/supervisor/supervisor': 'supervisor',
      '/supervisor/dist': 'dist'
    }
  }
}
