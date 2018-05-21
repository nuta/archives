const path = require('path')
const { app, protocol, BrowserWindow } = require('electron')
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

  // Allow absolute paths of file:// scheme.
  protocol.interceptFileProtocol('file', function(req, callback) {
    let abspath = req.url.substr(7)
    if (abspath === '/') {
      abspath = '/index.html'
    }

    callback(path.normalize(path.join(__dirname, '..', abspath)))
  })

  const windowURL = (process.env.NODE_ENV === 'development')
    ? 'http://localhost:9080' : `file:///`

  if (process.env.NODE_ENV === 'development' || process.env.ENABLE_DEV_TOOLS) {
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

