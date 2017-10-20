const fs = require('fs')
const path = require('path')
const find = require('./find')
const mkdirp = require('./mkdirp')

module.exports = (srcDir, destDir, ignorePatterns = []) => {
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

  for (let i = 0; i < files.length; i++) {
    const filepath = files[i]

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
