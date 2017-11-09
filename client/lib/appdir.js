const fs = require('fs')
const path = require('path')
const yaml = require('js-yaml')

function loadAppYAML(appDir) {
  if (!fs.existsSync(appDir)) {
    throw new Error(`invalid app dir: ${appDir}`)
  }

  const appYAMLPath = path.join(appDir, 'app.yaml')
  if (!fs.existsSync(appYAMLPath)) {
    throw new Error(`app.yaml not found: ${appYAMLPath}`)
  }

  return yaml.safeLoad(fs.readFileSync(appYAMLPath))
}

module.exports = { loadAppYAML }
