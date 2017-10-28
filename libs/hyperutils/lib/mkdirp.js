const fs = require('fs')
const path = require('path')

module.exports = dir => {
  const dirs = dir.split('/')
  let dirpath = '/'

  for (const relpath of dirs) {
    dirpath = path.join(dirpath, relpath)

    if (!fs.existsSync(dirpath)) {
      fs.mkdirSync(dirpath)
    }
  }
}
