const fs = require('fs')
const path = require('path')

module.exports = (basedir) => {
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
