const fs = require('fs')
const path = require('path')
const { spawnSync } = require('child_process')

function find(basedir) {
  let files = []
  let dirs = [basedir]

  if (fs.statSync(basedir).isFile()) {
    return [basedir]
  }

  do {
    const currentDir = dirs.pop()
    const ls = fs.readdirSync(currentDir)
    for (const relpath of ls) {
      const filepath = path.join(currentDir, relpath)
      if (fs.statSync(filepath).isDirectory()) {
        dirs.push(filepath)
      } else {
        files.push(path.relative(basedir, filepath))
      }
    }
  } while (dirs.length > 0)

  return files
}

function mkdirp(dir) {
  const dirs = path.resolve(dir).split('/')
  let dirpath = '/'

  for (const relpath of dirs) {
    dirpath = path.join(dirpath, relpath)

    if (!fs.existsSync(dirpath)) {
      fs.mkdirSync(dirpath)
    }
  }
}

function copyFiles(srcDir, destDir, ignorePatterns = []) {
  const files = find(srcDir)

  if (fs.statSync(srcDir).isFile()) {
    if (fs.existsSync(destDir) && fs.statSync(destDir).isDirectory()) {
      fs.copyFileSync(srcDir, path.join(destDir, path.basename(srcDir)))
    } else {
      mkdirp(path.dirname(path.resolve(destDir)))
      fs.copyFileSync(srcDir, destDir)
    }
    return
  }

  for (const filepath of files) {
    if (ignorePatterns.some(pattern => path.join(srcDir, filepath).match(pattern))) {
      continue
    }

    mkdirp(path.dirname(path.resolve(destDir, `./${filepath}`)))
    fs.copyFileSync(
      path.resolve(srcDir, `./${filepath}`),
      path.resolve(destDir, `./${filepath}`)
    )
  }
}

function shasum(filepath) {
  const cp = spawnSync('shasum', ['-a', '256', filepath], {
    stdio: 'pipe',
    encoding: 'utf-8'
  })

  if (cp.error) { throw new Error(cp.error) }

  return cp.stdout.split(' ')[0]
}

function download(pkg, sha256) {
  const filepath = path.resolve(build.downloadsDir, pkg.name, path.basename(pkg.url))
  mkdirp(path.dirname(filepath))

  if (fs.existsSync(filepath) && shasum(filepath) === sha256) {
    // Use the already downloaded file
    return filepath
  }

  spawnSync('curl', ['-fSLo', filepath, pkg.url], { stdio: 'inherit' })
  const computed = shasum(filepath)
  if (computed !== sha256) {
    throw new Error(`shasum mismatch: ${filepath} (${computed})`)
  }

  return filepath
}

function extract(filepath, dest) {
  if (fs.existsSync(dest)) { return }

  if (filepath.match(/\.tar\.(xz|bz2?|gz)$/)) {
    mkdirp(dest)
    spawnSync('tar', ['xf', filepath, '-C', dest, '--strip-components', '1'], { stdio: 'inherit' })
  } else if (filepath.match(/\.zip$/)) {
    const tempDest = dest + '-tmp'
    spawnSync('unzip', [filepath, '-d', tempDest], { stdio: 'inherit' })

    // --strip-components 1.
    spawnSync('mv', [path.join(tempDest, fs.readdirSync(tempDest)[0]), dest], { stdio: 'inherit' })
  } else if (filepath.match(/\.deb$/)) {
    mkdirp(dest)
    const cwd = process.cwd()
    process.chdir(dest)
    spawnSync('ar', ['x', filepath], { stdio: 'inherit' })
    spawnSync('tar', ['xf', 'data.tar.xz'], { stdio: 'inherit' })
    process.chdir(cwd)
  } else {
    throw new Error(`unsupported file type: ${filepath}`)
  }
}

function rmrf(filepath) {
  if (!fs.existsSync(filepath)) {
      return;
  }

  const remaining = [filepath];
  while (true) {
      const target = remaining.pop();
      if (!target) {
          break
      }

    if (fs.statSync(target).isDirectory()) {
          const innerFiles = fs.readdirSync(target).map((name) => path.join(target, name));
          if (innerFiles.length === 0) {
              fs.rmdirSync(target);
          } else {
              remaining.concat(innerFiles);
          }
      } else {
          // A normal file.
          fs.unlinkSync(target);
      }
  }
}

module.exports = { find, mkdirp, copyFiles, shasum, download, extract, rmrf }
