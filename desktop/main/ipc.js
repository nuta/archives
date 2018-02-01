const path = require('path')
const { ipcMain } = require('electron')
const makestack = require('makestack-sdk')

function initIpc() {
  ipcMain.on('loadCredentials', (event, arg) => {
    event.returnValue = makestack.config.loadCredentials()
  })

  ipcMain.on('saveCredentials', (event, arg) => {
    makestack.config.saveCredentials(arg)
  })

  ipcMain.on('getAvailableDrives', (event, arg) => {
    makestack.drive.getAvailableDrives().then(drives => {
      event.returnValue = drives
    })
  })

  ipcMain.on('install', async (event, arg) => {
    const flashCommand = [process.argv0, path.resolve(__dirname, 'flasher.js')]

    try {
      await makestack.install({
        deviceName: arg.deviceName,
        deviceType: arg.deviceType,
        osType: arg.os,
        adapter: arg.adapter,
        drive: arg.drive,
        ignoreDuplication: arg.ignoreDuplication,
        flashCommand,
        wifiSSID: arg.wifiSSID,
        wifiPassword: arg.wifiPassword,
        wifiCountry: arg.wifiCountry
      }, (stage, state) => {
        event.sender.send('progress', stage, state)
      })
    } catch (e) {
      const message = (e.constructor.name === 'FatalError') ? e.message : e.stack
      event.sender.send('error', message)
    }
  })
}

module.exports = {
  initIpc
}
