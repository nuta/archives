const fs = require('fs')
const os = require('os')
const path = require('path')

module.exports = class {
  updateOS(imagePath) {
    fs.renameSync(imagePath, path.join(os.homedir(), '.makestack/mock-os-update-test.img'))
  }
}
