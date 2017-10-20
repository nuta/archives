const fs = require('fs')

module.exports = class {
  updateOS(imagePath) {
    fs.renameSync(imagePath, '/boot/vmlinuz')
  }
}
