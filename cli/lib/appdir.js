const os = require('os')
const fs = require('fs')
const path = require('path')

function loadAppJSON(appDir) {
  if (!fs.existsSync(appDir)) {
    throw new Error(`invalid app dir: ${appDir}`)
  }

  const appJsonPath = path.join(appDir, 'app.json')
  if (!fs.existsSync(appJsonPath)) {
    throw new Error(`app.json not found: ${appJsonPath}`)
  }

  return JSON.parse(fs.readFileSync(appJsonPath))
}

module.exports = { loadAppJSON }
