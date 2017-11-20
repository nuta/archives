import * as fs from 'fs';

module.exports = class {
  updateOS(imagePath) {
    fs.renameSync(imagePath, '/boot/vmlinuz')
  }
}
