const path = require('path')
const {
  run, buildPath, isNewerDirContent, loadJsonFile, saveJsonFile
} = require('../pkgbuilder').pkg

const ignore = [/^node_modules/]
const supervisorPath = path.resolve(__dirname, '../../supervisor')
const runtimePath = path.resolve(__dirname, '../../runtime')

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
    let packageJson = loadJsonFile('./package.json')
    packageJson.dependencies['@makestack/runtime'] = 'file:' + runtimePath
    saveJsonFile('package.json', packageJson)

    run(['yarn', '--no-progress'])
  },

  rootfs() {
    return {
      '/supervisor/package.json': 'package.json',
      '/supervisor/supervisor': 'supervisor',
      '/supervisor/dist': 'dist'
    }
  }
}
