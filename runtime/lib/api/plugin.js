const path = require('path')

/* Loads a plugin */
function plugin(name) {
  if (!name.match(/^([a-zA-Z0-9\-_]+\/)?[a-zA-Z0-9\-_]+$/)) {
    throw new Error(`invalid plugin name: ${name}`)
  }

  // This file will be installed at <appdir>/node_modules/runtime/lib/plugin.js
  const appDir = path.resolve(__dirname, '../../..')
  return require(`${appDir}/plugins/${name}`)
}

module.exports = plugin
