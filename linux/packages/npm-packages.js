const fs = require('fs')
const path = require('path')
const {
  config, run, assetPath, isNewerFile, loadJsonFile, saveJsonFile,
  copyFiles, progress
} = require('../pkgbuilder').pkg

const localPackages = {
  '@makestack/runtime': path.resolve(__dirname, '../../runtime')
}

const pluginsDir = path.resolve(__dirname, '../../plugins')
for (const pluginName of fs.readdirSync(pluginsDir)) {
  const pluginDir = path.join(pluginsDir, pluginName)
  if (fs.statSync(pluginDir).isDirectory()) {
    localPackages[`@makestack/${pluginName}`] = pluginDir
  }
}

module.exports = {
  name: 'npm-packages',
  type: 'library',
  path: assetPath('npm-packages'),

  changed() {
    return isNewerFile('package.json', assetPath('npm-packages', 'package.json'))
  },

  build() {
    // Build npm packages in this repository. Native modules will be
    // replaced by `npm rebuild`.
    const buildDir = 'build'
    for (const dir of Object.values(localPackages)) {
      const pluginDir = path.join(buildDir, path.basename(dir))
      copyFiles(dir, pluginDir, [/(node_modules|dist|build)/])
      progress(`Building ${pluginDir}`)
      run(['yarn', '--no-progress'], {}, pluginDir)
    }

    // Add local packages to dependencies.
    progress('Manipulating package.json')
    const packageJson = loadJsonFile('./package.json')
    for (const [name, dir] of Object.entries(localPackages)) {
      const pluginDir = path.join(buildDir, path.basename(dir))
      packageJson.dependencies[name] = `file:${pluginDir}`
    }
    saveJsonFile('package.json', packageJson)

    // Copy local pacakges and external npm packages and build native modules.
    progress('Installing npm packages')
    run(['yarn', '--no-progress'])
    run(['npm', 'rebuild', '--arch', config('target.node_gyp_arch')], {
      CC: `${config('target.toolchain_prefix')}gcc`,
      CXX: `${config('target.toolchain_prefix')}g++`,
      LINK: `${config('target.toolchain_prefix')}g++`
    })

    // Remove dependencies from MakeStack npm packages except runtimeDependencies.
    progress('Removing packages except runtimeDependencies')
    for (const dir of Object.values(localPackages)) {
      const pluginDir = path.join(buildDir, path.basename(dir))
      const packageJsonPath = path.join(pluginDir, 'package.json')

      const packageJson = loadJsonFile(packageJsonPath)
      packageJson['dependencies'] = packageJson['runtimeDependencies']
      saveJsonFile(packageJsonPath, packageJson)
    }
    run(['yarn', '--no-progress', '--ignore-scripts', '--production'])
  },

  rootfsIgnore: [/\/node_modules\/.+\/node_modules/],
  rootfs() {
    // Note that the destination dir must be $node_prefix/lib/node_modules
    // where $node_prefix is `process.config.variables.node_prefix`.
    return {
      '/lib/node_modules': 'node_modules'
    }
  }
}
