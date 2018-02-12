const path = require('path')
const { app, BrowserWindow } = require('electron')
const { initIpc } = require('./ipc')

let mainWindow
function createWindow() {
  initIpc()

  mainWindow = new BrowserWindow({
    height: 800,
    width: 1000,
    titleBarStyle: 'hidden',
    title: 'MakeStack',
    webPreferences: {
      // TODO: enable Context Isolation
      nodeIntegration: false,
      preload: path.resolve(__dirname, 'preload.js')
    }
  })

  const windowURL = (process.env.NODE_ENV === 'development')
    ? 'http://localhost:9080' : `file://${__dirname}/../ui/index.html`

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

