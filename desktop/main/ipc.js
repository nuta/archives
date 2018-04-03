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
        app: arg.app,
        adapter: arg.adapter,
        drive: arg.drive,
        flashCommand,
        wifiSSID: arg.wifiSSID,
        wifiPassword: arg.wifiPassword,
        wifiCountry: arg.wifiCountry
      }, (stage, state) => {
        event.sender.send('installProgress', stage, state)
      })
    } catch (e) {
      event.sender.send('installError', e.stack)
    }
  })
}

module.exports = {
  initIpc
}
