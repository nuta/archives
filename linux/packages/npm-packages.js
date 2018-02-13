// TODO: Refactoring
const fs = require('fs')
const path = require('path')
const {
  config, run, assetPath, isNewerFile, find,
  progress, isNewerDirContent, loadJsonFile,
  rootfsPath
} = require('../pkgbuilder').pkg

const binPath = path.resolve(__dirname, '../pkgbuilder/node_modules/.bin')
const PATH = `${process.env.PATH}:${binPath}`
const packageJson = loadJsonFile(assetPath('npm-packages', 'package.json'))
const unnecessaryDependencies = ['nan', '@types']
const localPackages = {}
for (const [name, dep] of Object.entries(packageJson.dependencies)) {
  if (dep.startsWith('file:')) {
    localPackages[name] = path.resolve(__dirname, dep.replace('file:../../', ''))
  }
}

function transpilePackages() {
  for (const name of Object.keys(localPackages)) {
    run(['tsc'], { PATH }, path.join('node_modules', name))
  }
}

function buildPackages() {
  for (const filepath of find('node_modules').map(f => path.join('node_modules', f))) {
    if (filepath.match(/node_modules\/.+\/package.json$/)) {
      const packageJson = loadJsonFile(filepath)
      if (packageJson.gypfile) {
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
  const unnecessaryPackages = [
    '@types',
    'nan'
  ]

  for (const name of unnecessaryPackages) {
    run(['rm', '-rf', path.join('node_modules', name)])
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

    for (const [name, dir] of Object.entries(localPackages)) {
      if (isNewerDirContent(dir, rootfsPath(`lib/node/${name}`), [/(node_modules|dist|build)/])) {
        return true
      }
    }

    return false
  },

  build() {
    // `node_modules' in local packages in this repository will be copied by yarn
    // and it could be accidentally bundled to the rootfs since yarn creates node_modules
    // unless --flat option.
    progress('Removing node_modules from local packages')
    for (const dir of Object.values(localPackages)) {
      progress(`Removing ${dir}/node_modules`)
      run(['rm', '-rf', path.join(dir, 'node_modules')])
    }

    progress('Installing npm packages')
    run(['yarn', '--no-progress', '--ignore-scripts'])

    progress('Transpling local packages')
    transpilePackages()

    progress('Building native modules')
    buildPackages()

    progress('Removing unnecessary packages')
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
