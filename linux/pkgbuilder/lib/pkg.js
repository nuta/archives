const fs = require('fs')
const os = require('os')
const path = require('path')
const { spawnSync } = require('child_process')
const { find, mkdirp } = require('./helpers')
const chalk = require('chalk')

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
  for (const file of files) {
    if (ignorePatterns.some(pattern => file.match(pattern))) {
      continue
    }

    const file1 = path.join(dir1, file)
    const file2 = path.join(dir2, file)
    if (!fs.existsSync(file2)) { return true }

    const stat1 = fs.statSync(file1)
    const stat2 = fs.statSync(file2)
    if (stat1.ctimeMs > stat2.ctimeMs) { return true }
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
  console.log(chalk.bold(`${argv.join(' ')}`))
  const cp = spawnSync(argv[0], argv.slice(1), {
    stdio: 'inherit',
    cwd: cwd || process.cwd(),
    env: Object.assign({
      PATH: process.env.PATH,
      MAKEFLAGS: `-j${os.cpus().length}`
    }, env)
  })

  if (cp.error) {
    throw new Error(`error: failed to run ${argv[0]}: ${cp.error}`)
  }

  if (cp.status !== 0) {
    throw new Error(`error: \`${argv[0]}' exited with ${cp.status}.`)
  }
}

function runWithPipe(argv, env = {}, cwd = process.cwd(), options = {}) {
  console.log(chalk.bold(`${argv.join(' ')}`))
  const cp = spawnSync(argv[0], argv.slice(1), Object.assign({
    cwd,
    encoding: 'utf-8',
    env: Object.assign({
      PATH: process.env.PATH,
      MAKEFLAGS: `-j${os.cpus().length}`
    }, env)
  }, options))

  if (cp.error) {
    throw new Error(`error: failed to run ${argv[0]}: ${cp.error}`)
  }

  if (cp.status !== 0) {
    throw new Error(
      `error: \`${argv[0]}' exited with ${cp.status}.\n` +
      `stdout:${cp.stdout}\n\nstderr:${cp.stderr}`
    )
  }

  return cp.stdout
}

function sudo(argv, env) {
  console.log(chalk.bold(`sudo ${argv.join(' ')}`))
  spawnSync('sudo', argv, {
    stdio: 'inherit',
    env: Object.assign({
      PATH: process.env.PATH,
      MAKEFLAGS: `-j${os.cpus().length}`
    }, env)
  })
}

function buildFatImage(imageFile) {

  const mountPoint = buildPath('image')
  const username = runWithPipe(['whoami']).replace('\n', '')

  mkdirp(mountPoint)
  run(['dd', 'if=/dev/zero', `of=${imageFile}`, 'bs=1M', 'count=64'])
  runWithPipe(['fdisk', imageFile], null, null, { input: 'n\np\n\n\n\na\n1\nw\n' })

  const partedOutput = runWithPipe(
    ['parted', '-s', imageFile, 'unit', 'b', 'print']
  ).split('\n').filter(line => line.includes('boot'))[0]

  if (!partedOutput) {
    throw new Error('failed to get the partiton layout by parted(8');
  }

  const partOffset = partedOutput.split(/\s+/)[1].replace(/B$/, '')
  const partLength = partedOutput.split(/\s+/)[3].replace(/B$/, '')

  const loopFile = runWithPipe(
    ['sudo', 'losetup', '-o', partOffset, '--sizelimit', partLength,
    '--show', '-f', imageFile]
  ).replace(/\n+$/, '')

  sudo(['mkfs.fat', '-n', 'MAKESTACK', loopFile])
  run(['mkdir', '-p', mountPoint])
  sudo(['mount', loopFile, mountPoint, '-o', `uid=${username}`, '-o', `gid=${username}`])
  run(['sh', '-c', `cp -r ${bootfsPath('.')}/* ${mountPoint}`])
  sudo(['umount', mountPoint])
  run(['sudo', 'losetup', '-d', loopFile])
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
  runWithPipe,
  sudo,
  mkdirp,
  buildFatImage
}
