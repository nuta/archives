const path = require('path')
const fs = require('fs')
const os = require('os')
const { ipcRenderer } = require('electron')

window.ipcRenderer = ipcRenderer

window._appendToLog = (message) => {
  fs.appendFileSync(
    path.resolve(os.homedir(), '.makestack/desktop.log'),
    message
  )
}

window.eval = global.eval = function() {
  throw new Error(`Eval is not available.`)
}

window.require = (moduleName) => {
  const whitelistedModules = [
    'stream', 'querystring', 'timers'
  ]

  if (whitelistedModules.includes(moduleName)) {
    return require(moduleName)
  } else {
    throw new Error(`${moduleName} is not allowed to load.`)
  }
}
