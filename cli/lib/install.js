const path = require('path')
const os = require('os')
const fs = require('fs')
const fetch = require('node-fetch')
const ipc = require('node-ipc')
const quote = require('shell-quote').quote
const sudo = require('sudo-prompt')
const { createFile, generateTempPath,
  generateRandomString } = require('hyperutils')
const api = require('./api')
const { getDriveSize } = require('./drive')
const { getLatestGitHubRelease } = require('./github_releases')

function replaceBuffer(buf, value, id) {
  const needle = `_____REPLACE_ME_MAKESTACK_CONFIG_${id}_____`

  const index = buf.indexOf(Buffer.from(needle))
  if (index === -1) {
    throw new Error(`replaceBuffer: failed to replace ${id}`)
  }

  let paddedValue = Buffer.alloc(needle.length, ' ')
  let valueBuf = Buffer.from(value)
  valueBuf.copy(paddedValue)
  paddedValue.copy(buf, index)
  return buf
}

async function registerOrGetDevice(name, type, ignoreDuplication) {
  let device
  try {
    device = await api.registerDevice(name, type)
  } catch (e) {
    // FIXME: add an option to accept 4xx errors
    if (ignoreDuplication && e.message.includes("name: [ 'has already been taken' ]")) {
      // There is already a device with same name.
      device = await api.getDevice(name)
    } else {
      throw new Error('failed to register/fetch the device')
    }
  }

  return device
}

async function downloadDiskImage(osType, deviceType) {
  const [version, osImageURL] = await getLatestGitHubRelease('seiyanuta/makestack',
    `makestack-${osType}-`, `${deviceType}.img`)
  const basename = path.basename(osImageURL)
  const orignalImage = path.join(process.env.HOME, `.makestack/caches/${basename}`)
  createFile(orignalImage, await (await fetch(osImageURL)).buffer())
  return [version, orignalImage]
}

function writeConfigToDiskIamge(osVersion, type, orignalImage, device, adapter) {
  const imagePath = generateTempPath()

  // TODO: What if the image is large?
  let image = fs.readFileSync(orignalImage)
  image = replaceBuffer(image, type, 'DEVICE_TYPE')
  image = replaceBuffer(image, osVersion, 'OS_VERSION')
  image = replaceBuffer(image, device.device_id, 'DEVICE_ID')
  image = replaceBuffer(image, device.device_secret, 'DEVICE_SECRET')
  image = replaceBuffer(image, api.serverURL, 'SERVER_URL_abcdefghijklmnopqrstuvwxyz1234567890')
  image = replaceBuffer(image, adapter, 'ADAPTER')
  fs.writeFileSync(imagePath, image)

  return imagePath
}

function prepareFlashCommand(flashCommand, ipcPath, drive, driveSize, imagePath) {
  let prefix = 'env '
  const env = {
    DRIVE: drive,
    IMAGE_WRITER: 'y',
    DRIVE_SIZE: driveSize,
    IMAGE_PATH: imagePath,
    IPC_PATH: ipcPath,
    ELECTRON_RUN_AS_NODE: '1'
  }

  for (let name in env) {
    prefix += `${name}=${quote([env[name]])} `
  }

  return prefix + quote(flashCommand)
}

function flash(flashCommand, drive, driveSize, imagePath, progress) {
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

    const command = prepareFlashCommand(flashCommand, ipcPath, drive, driveSize, imagePath)
    const options = { name: 'MakeStack Installer' }
    sudo.exec(command, options, (error, stdout, stderr) => {
      if (error) { reject(error) }

      ipc.server.stop()
      resolve()
    })
  })
}

module.exports = async ({ deviceName, deviceType, osType, adapter, drive, ignoreDuplication, flashCommand }, progress) => {
  progress('look-for-drive')
  const driveSize = await getDriveSize(drive)
  progress('register')
  const device = await registerOrGetDevice(deviceName, deviceType, ignoreDuplication)
  progress('download')
  const [osVersion, orignalImage] = await downloadDiskImage(osType, deviceType)
  progress('config')
  const imagePath = writeConfigToDiskIamge(osVersion, deviceType, orignalImage, device, adapter)
  progress('flash')
  await flash(flashCommand, drive, driveSize, imagePath, progress)
  progress('success')
}
