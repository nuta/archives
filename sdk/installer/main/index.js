const path = require('path')
const { app, BrowserWindow, ipcMain } = require('electron')
const makestack = require('makestack')

let mainWindow
function createWindow() {
  mainWindow = new BrowserWindow({
    height: 620,
    width: 450
  })

  const windowURL = (process.env.NODE_ENV === 'development')
    ? 'http://localhost:9080' : `file://${__dirname}/../renderer/index.html`

  if (process.env.NODE_ENV === 'development') {
    mainWindow.webContents.openDevTools({ mode: 'detach' })
  }

  mainWindow.loadURL(windowURL)

  mainWindow.on('closed', () => {
    mainWindow = null
  })
}

app.on('ready', createWindow)

app.on('window-all-closed', () => {
  if (process.platform !== 'darwin') {
    app.quit()
  }
})

app.on('activate', () => {
  if (mainWindow === null) {
    createWindow()
  }
})

ipcMain.on('getAvailableDrives', (event, args) => {
  makestack.drive.getAvailableDrives().then(drives => {
    event.returnValue = drives
  })
})

ipcMain.on('install', async(event, args) => {
  const flashCommand = [process.argv0, path.resolve(__dirname, 'flasher.js')]

  try {
    await makestack.install({
      deviceName: args.deviceName,
      deviceType: args.deviceType,
      osType: args.os,
      adapter: args.adapter,
      drive: args.drive,
      ignoreDuplication: args.ignoreDuplication,
      flashCommand,
      wifiSSID: args.wifiSSID,
      wifiPassword: args.wifiPassword,
      wifiCountry: args.wifiCountry
    }, (stage, state) => {
      event.sender.send('progress', stage, state)
    })
  } catch (e) {
    const message = (e.constructor.name === 'FatalError') ? e.message : e.stack
    event.sender.send('error', message)
  }
})
