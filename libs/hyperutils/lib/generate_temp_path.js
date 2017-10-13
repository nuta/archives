const os = require('os')
const path = require('path')
const generateRandomString = require('./generate_random_string')
module.exports = () => {
  return path.join(os.tmpdir(), generateRandomString(32) + '.img')
}
