const fs = require('fs')
const path = require('path')
const child_process = require('child_process')
const { find, mkdirp } = require('hyperutils')

function config(key) {
  if (!(key in build.config)) {
    throw new Error(`undefined config \`${key}'`)
  }

  return build.config[key]
}

function isNewerFile(file1, file2) {
  if (!fs.existsSync(file1) || !fs.existsSync(file2)) {
    return false
  }

  const stat1 = fs.statSync(file1)
  const stat2 = fs.statSync(file2)

  return stat1.ctimeMs > stat2.ctimeMs
}

function isNewerDirContent(dir1, dir2, ignorePatterns) {

  if (!fs.existsSync(dir2)) {
    return true
  }

  let files = find(dir1)
  for (let i = 0; i < files.length; i++) {
    if (ignorePatterns.some(pattern => files[i].match(pattern))) {
      continue
    }

    const file1 = path.join(dir1, files[i])
    const file2 = path.join(dir2, files[i])
    if (!fs.existsSync(file2))
      return true

    const stat1 = fs.statSync(file1)
    const stat2 = fs.statSync(file2)
    if (stat1.ctimeMs > stat2.ctimeMs)
      return true
  }

  return false
}

function copyFile(src, dest) {
  mkdirp(path.dirname(dest))
  return fs.copyFileSync(src, dest)
}

function assetPath(pkgName, filepath) {
  return path.resolve(__dirname, '../../packages', pkgName, filepath)
}

function buildPath(filepath) {
  return path.resolve(build.buildDir, filepath)
}

function bootfsPath(filepath) {
  return path.resolve(build.bootfsDir, filepath)
}

function rootfsPath(filepath) {
  return path.resolve(build.rootfsDir, filepath)
}

function isRebuilt(pkg) {
  return build.rebuiltPackages.includes(pkg)
}

function run(argv, env, cwd) {
  console.log(`+++ ${argv.join(' ')}`)
  const cp = child_process.spawnSync(argv[0], argv.slice(1), {
    stdio: 'inherit',
    cwd: cwd || process.cwd(),
    env: Object.assign({
      PATH: process.env.PATH,
      MAKEFLAGS: '-j4' // FIXME
    }, env)
  })

  if (cp.error) {
    throw new Error(`error: a child process returned \`${cp.error.code}'`)
  }
}

function sudo(argv, env) {
  console.log(`+++ sudo ${argv.join(' ')}`)
  child_process.spawnSync('sudo', argv, {
    stdio: 'inherit',
    env: Object.assign({
      PATH: process.env.PATH,
      MAKEFLAGS: '-j4' // FIXME
    }, env)
  })
}

module.exports = {
  config,
  isNewerFile,
  isNewerDirContent,
  copyFile,
  assetPath,
  buildPath,
  bootfsPath,
  rootfsPath,
  isRebuilt,
  run,
  sudo,
  mkdirp
}
