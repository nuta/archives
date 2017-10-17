const fs = require('fs')
const path = require('path')
const mkdirp = require('./mkdirp')

module.exports = (filepath, body) => {
  mkdirp(path.dirname(filepath))
  fs.writeFileSync(filepath, body)
}
