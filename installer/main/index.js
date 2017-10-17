const path = require('path')
const { app, BrowserWindow, ipcMain } = require('electron')
const makestack = require('makestack')

let mainWindow
function createWindow() {
  mainWindow = new BrowserWindow({
    height: 500,
    width: 450,
    useContentSize: true
  })

  const windowURL = (process.env.NODE_ENV === 'development')
    ? 'http://localhost:9080' : `file://${__dirname}/../renderer/index.html`

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
  await makestack.install(
    args.deviceName, args.deviceType, args.os,
    args.adapter, args.drive, args.ignoreDuplication,
    flashCommand, (stage, state) => {
      event.sender.send('progress', stage, state)
    })
})
