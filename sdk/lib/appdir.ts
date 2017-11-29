import fs = require('fs');
import path = require('path');
import yaml = require('js-yaml');

export function loadAppYAML(appDir) {
  if (!fs.existsSync(appDir)) {
    throw new Error(`invalid app dir: ${appDir}`)
  }

  const appYAMLPath = path.join(appDir, 'app.yaml')
  if (!fs.existsSync(appYAMLPath)) {
    throw new Error(`app.yaml not found: ${appYAMLPath}`)
  }

  return yaml.safeLoad(fs.readFileSync(appYAMLPath)) || {}
}
