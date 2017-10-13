const path = require('path')
const url = require('url')
const { app, BrowserWindow, ipcMain } = require('electron')
const makestack = require('makestack')

let mainWindow
function createWindow () {
  mainWindow = new BrowserWindow({
    height: 500,
    width: 450,
    useContentSize: true
  })

  mainWindow.loadURL(url.format({
    pathname: path.join(__dirname, '../renderer/index.html'),
    protocol: 'file:',
    slashes: true
  }))

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

ipcMain.on('install', async (event, args) => {
  const flashCommand = [process.argv0, path.resolve(__dirname, 'flasher.js')]
  await makestack.install(
    args.deviceName, args.deviceType, args.os,
    args.adapter, args.drive, args.ignoreDuplication,
    flashCommand, (stage, state) => {

      event.sender.send('progress', stage, state)
  })
})