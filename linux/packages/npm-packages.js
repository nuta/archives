const fs = require('fs')
const path = require('path')
const {
  config, run, assetPath, isNewerFile, find,
  progress, isNewerDirContent, loadJsonFile,
  saveJsonFile, rootfsPath
} = require('../pkgbuilder').pkg

const binPath = path.resolve(__dirname, '../pkgbuilder/node_modules/.bin')
const PATH = `${process.env.PATH}:${binPath}`

// Npm packages in this repository.
const localPackages = {
  '@makestack/runtime': {
    srcDir: path.resolve(__dirname, '../../runtime'),
  }
}

// Look for plugins in this repository and add them to `localPackages`.
const pluginsDir = path.resolve(__dirname, '../../plugins')
for (const pluginName of fs.readdirSync(pluginsDir)) {
  const srcDir = path.join(pluginsDir, pluginName)

  if (fs.statSync(srcDir).isDirectory()) {
    localPackages[`@makestack/${pluginName}`] = { srcDir }
  }
}

function generatePackageJson() {
  const packageJson = loadJsonFile('package.json')

  for (const [name, { srcDir }] of Object.entries(localPackages)) {
    packageJson.dependencies[name] = `file:${srcDir}`
  }

  saveJsonFile('package.json', packageJson)
}

function transpilePackages() {
  for (const name of Object.keys(localPackages)) {
    progress(`Transpiling ${name}`)
    run(['tsc'], { PATH }, path.join('node_modules', name))
  }
}

function buildPackages() {
  for (const filepath of find('node_modules').map(f => path.join('node_modules', f))) {
    if (filepath.match(/node_modules\/.+\/package.json$/)) {
      const packageJson = loadJsonFile(filepath)
      if (packageJson.gypfile) {
        progress(`Building ${packageJson.name}`)
        run(['node-gyp', 'rebuild', '--loglevel=warn', '--arch', config('target.node_gyp_arch')], {
          PATH,
          CC: `${config('target.toolchain_prefix')}gcc`,
          CXX: `${config('target.toolchain_prefix')}g++`,
          LINK: `${config('target.toolchain_prefix')}g++`
        }, path.dirname(filepath))
      }
    }
  }
}

function removeUnnecessaryPackages() {
  const npmDependencies = ['@makestack']
  for (const name of fs.readdirSync('node_modules')) {
    if (!npmDependencies.includes(name)) {
      progress(`Removing node_modules/${name}`)
      run(['rm', '-rf', path.join('node_modules', name)])
    }
  }
}

module.exports = {
  name: 'npm-packages',
  type: 'library',
  path: assetPath('npm-packages'),

  changed() {
    if (isNewerFile('package.json', assetPath('npm-packages', 'package.json'))) {
      return true
    }

    for (const [name, { srcDir }] of Object.entries(localPackages)) {
      if (isNewerDirContent(srcDir, rootfsPath(`lib/node/${name}`), [/(node_modules|dist|build)/])) {
        return true
      }
    }

    return false
  },

  build() {
    progress('Removing node_modules from local packages')
    for (const { srcDir } of Object.values(localPackages)) {
      progress(`Removing ${srcDir}/node_modules`)
      run(['rm', '-rf', path.join(srcDir, 'node_modules')])
    }

    progress('Generating package.json')
    generatePackageJson()

    progress('Installing npm packages')
    run(['yarn', '--no-progress', '--ignore-scripts'])

    transpilePackages()
    buildPackages()
    removeUnnecessaryPackages()
  },

  rootfs() {
    // Note that the destination dir must be $node_prefix/lib/node
    // where $node_prefix is `process.config.variables.node_prefix`.
    return {
      '/lib/node': 'node_modules'
    }
  }
}
