const fs = require('fs')
const imageWrite = require('etcher-image-write')
const mountutils = require('mountutils')
const ipc = require('node-ipc')

/*
 *  Be careful! This command will be executed in root!!!
 */

function connectIPC(ipcPath) {
  return new Promise((resolve, reject) => {
    ipc.config.retry = 5
    ipc.config.maxRetries = 3
    ipc.config.logger = () => { }
    ipc.connectTo('server', ipcPath, () => {
      ipc.of.server.on('connect', () => resolve(ipc.of.server))
    })
  })
}

function umount(drive) {
  return new Promise((resolve, reject) => {
    mountutils.unmountDisk(drive, (error) => {
      if (error) { reject(error) } else { resolve() }
    })
  })
}

module.exports = async() => {
  const ipcPath = process.env.IPC_PATH
  const drive = process.env.DRIVE
  const driveSize = parseInt(process.env.DRIVE_SIZE)
  const imagePath = process.env.IMAGE_PATH

  if (ipcPath === undefined || drive === undefined ||
    driveSize === undefined || imagePath === undefined) {
    throw new Error("specify `IPC_PATH', `DRIVE_PATH', `DRIVE_SIZE' and `IMAGE_PATH'")
  }

  console.log('info:', drive, driveSize, imagePath)

  console.log('info:', `connecting to ${ipcPath}`)
  const server = await connectIPC(ipcPath)

  console.log('info:', 'unmounting')
  await umount(drive)

  console.log('info:', 'writing')
  const writer = imageWrite.write({
    fd: fs.openSync(drive, 'rs+'),
    device: drive,
    size: driveSize
  }, {
    stream: fs.createReadStream(imagePath),
    size: fs.statSync(imagePath).size
  }, {
    check: true
  })

  writer.on('progress', state => {
    server.emit('progress', JSON.stringify(state))
  })

  writer.on('done', results => {
    server.emit('success', JSON.stringify(results))
    ipc.disconnect('server')
  })

  writer.on('error', error => {
    server.emit('error', error)
    ipc.disconnect('server')
  })
}
