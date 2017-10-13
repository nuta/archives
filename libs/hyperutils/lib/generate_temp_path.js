const os = require('os')
const path = require('path')

module.exports = () => {
  const random = Math.random().toString().substring(2)
  return path.join(os.tmpdir(), random + '.img')
}
