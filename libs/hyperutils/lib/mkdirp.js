const fs = require('fs')
const path = require('path')

module.exports = dir => {
  const dirs = dir.split(path.delimiter)
  let dirpath = ''
  
  for (let i = 0; i < dirs.length; i++) {
    dirpath = path.join(dirpath, dirs[i])

    try {
      fs.mkdirSync(dirpath)
    } catch (e) {
      if (e.code != 'EEXIST')
        throw e;  
    }
  }
}