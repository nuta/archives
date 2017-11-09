const fs = require('fs')
const os = require('os')
const path = require('path')

module.exports = class {
  updateOS(imagePath) {
    console.log('os image:', fs.readFileSync(imagePath, { encoding: 'utf-8' }))
    fs.renameSync(imagePath, path.join(os.homedir(), '.makestack/mock-os-update-test.img'))
  }
}
