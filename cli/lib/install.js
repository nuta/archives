const child_process = require('child_process')
const path = require('path')
const os = require('os')
const fs = require('fs')
const chalk = require('chalk')
const drivelist = require('drivelist')
const fetch = require('node-fetch')
const ipc = require('node-ipc')
const quote = require('shell-quote').quote
const sudo = require('sudo-prompt')
const { mkdirp, createFile, generateTempPath,
  generateRandomString } = require('hyperutils')
const api = require('./api')
const { getDriveSize } = require('./drive')

function replaceBuffer(buf, value, id) {
  const needle = `_____REPLACE_ME_MAKESTACK_CONFIG_${id}_____`

  const index = buf.indexOf(Buffer.from(needle))
  if (index == -1)
    throw `replaceBuffer: failed to replace ${id}`

  let paddedValue = Buffer.alloc(needle.length, ' ')
  let valueBuf = Buffer.from(value)
  valueBuf.copy(paddedValue)
  paddedValue.copy(buf, index)
  return buf
}

async function registerOrGetDevice(name, type, ignoreDuplication) {

  let device
  try {
    device = (await api.registerDevice(name, type)).json
  } catch (e) {
    if (ignoreDuplication && e.status == 422 && e.json.name == 'has already been taken') {
      device = (await api.getDevice(name)).json
    } else {
      throw `failed to register a device: server returned ${e.status}`
    }
  }

  return device
}

function getDiskImageURL(type, os) {
  // TODO
  return 'https://www.coins.tsukuba.ac.jp/~s1311386/kernel.img'
}

async function downloadDiskImage(type, os) {
  const osImageURL = getDiskImageURL()
  const basename = path.basename(osImageURL)
  const orignalImage = path.join(process.env.HOME, `.makestack/caches/${basename}`)
  createFile(orignalImage, await (await fetch(osImageURL)).buffer())
  return orignalImage
}

function writeConfigToDiskIamge(orignalImage, device, adapter) {
  const imagePath = generateTempPath()

  // TODO: What if the image is large?
  let image = fs.readFileSync(orignalImage)
  image = replaceBuffer(image, device.device_id, 'DEVICE_ID')
  image = replaceBuffer(image, device.device_secret, 'DEVICE_SECRET')
  image = replaceBuffer(image, api.serverURL, 'SERVER_URL_abcdefghijklmnopqrstuvwxyz1234567890')
  image = replaceBuffer(image, adapter, 'ADAPTER')
  fs.writeFileSync(imagePath, image)

  return imagePath
}

function prepareFlashCommand(ipcPath, drive, driveSize, imagePath) {
  let prefix = 'env '
  const env = {
    DRIVE: drive,
    IMAGE_WRITER: 'y',
    DRIVE_SIZE: driveSize,
    IMAGE_PATH: imagePath,
    IPC_PATH: ipcPath
  }

  for (let name in env) {
    prefix += `${name}=${quote([env[name]])} `
  }

  return prefix + quote(process.argv)
}

function flash(drive, driveSize, imagePath, progress) {
  return new Promise((resolve, reject) => {
    const ipcPath = path.join(os.tmpdir(),
      'makestack-installer' + generateRandomString(32))

    ipc.config.logger = () => { }
    ipc.serve(ipcPath, () => {
      ipc.server.on('progress', data => {
        progress('flashing', JSON.parse(data))
      })
    })
    ipc.server.start()

    const command = prepareFlashCommand(ipcPath, drive, driveSize, imagePath)
    const options = { name: 'MakeStack Installer' }
    sudo.exec(command, options, (error, stdout, stderr) => {
      if (error)
        reject(error)

      ipc.server.stop()
      resolve()
    })
  })
}

module.exports = async (name, type, os, adapter, drive, ignoreDuplication, progress) => {
  progress('look-for-drive')
  const driveSize = await getDriveSize(drive)
  progress('register')
  const device = await registerOrGetDevice(name, type, ignoreDuplication)
  progress('download')
  const orignalImage = await downloadDiskImage(type, os)
  progress('config')
  const imagePath = writeConfigToDiskIamge(orignalImage, device, adapter)
  progress('flash')
  await flash(drive, driveSize, imagePath, progress)
  progress('success')
}