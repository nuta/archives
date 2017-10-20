const fs = require('fs')
const path = require('path')

module.exports = dir => {
  const dirs = dir.split('/')
  let dirpath = '/'

  for (let i = 0; i < dirs.length; i++) {
    dirpath = path.join(dirpath, dirs[i])

    if (!fs.existsSync(dirpath)) {
      fs.mkdirSync(dirpath)
    }
  }
}
