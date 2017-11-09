const fs = require('fs')
const path = require('path')

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
  const dirs = dir.split('/')
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
    if (ignorePatterns.some(pattern => filepath.match(pattern))) {
      continue
    }

    mkdirp(path.dirname(path.resolve(destDir, `./${filepath}`)))
    fs.copyFileSync(
      path.resolve(srcDir, `./${filepath}`),
      path.resolve(destDir, `./${filepath}`)
    )
  }
}

module.exports = { find, mkdirp, copyFiles }
