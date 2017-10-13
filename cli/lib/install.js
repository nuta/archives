const child_process = require('child_process')
const path = require('path')
const fs = require('fs')
const chalk = require('chalk')
const drivelist = require('drivelist')
const fetch = require('node-fetch')
const { mkdirp, createFile, generateTempPath } = require('hyperutils')
const api = require('./api')

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

function getDriveSize(drive) {
  console.info(chalk.bold.blue("==> (1/5) Looking for the drive..."))

  return new Promise((resolve, reject) => {
    drivelist.list((error, drives) => {
      if (error)
        reject(error)

      for (let i = 0; i < drives.length; i++) {
        if (drives[i].device == drive)
          resolve(drives[i].size)
      }

      reject(`No such a drive: ${drive}`)
    })
  })
}

async function registerOrGetDevice(name, type, ignoreDuplication) {
  console.info(chalk.bold.blue("==> (2/5) Registering the device..."))

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

async function downloadDiskImage() {
  console.info(chalk.bold.blue("==> (3/5) Downloading the disk image..."))
  const osImageURL = 'https://www.coins.tsukuba.ac.jp/~s1311386/kernel.img'

  const basename = path.basename(osImageURL)
  const orignalImage = path.join(process.env.HOME, `.makestack/caches/${basename}`)
  createFile(orignalImage, await (await fetch(osImageURL)).buffer())
  return orignalImage
}

function writeConfigToDiskIamge(orignalImage, device, adapter) {
  console.info(chalk.bold.blue("==> (4/5) Writing configuration..."))
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

function flash(drive, driveSize, imagePath) {
  return new Promise((resolve, reject) => {
    console.info(chalk.bold.blue("==> (5/5) Flashing..."))
    const writer = child_process.spawn('sudo', ['-E'].concat(process.argv), {
      env: {
        IMAGE_WRITER: 'y',
        DRIVE: drive,
        DRIVE_SIZE: driveSize,
        IMAGE_PATH: imagePath
      }
    })

    writer.stdout.on('data', data => {
      let [type, json] = data.toString().split(': ', 2)
      if (type == 'info')
        return

      const state = JSON.parse(json)
      switch (type) {
        case 'progress':
          switch (state.type) {
            case 'write':
              console.info(`writing to the drive...(${state.percentage}%)`)
              break
            case 'check':
              console.info(`verifying...(${state.percentage}%)`)
              break
          }
          break

        case 'success':
          resolve()
          break

        case 'error':
          reject()
          break
      }
    })

    writer.on('exit', (code, signal) => {
      if (code != 0)
        reject('failed to flash')
    })
  })
}

module.exports = async (args, opts, logger) => {
  const driveSize = await getDriveSize(opts.drive)
  const device = await registerOrGetDevice(opts.name, opts.type, opts.ignoreDuplication)
  const orignalImage = await downloadDiskImage(opts.type, opts.os)
  const imagePath = writeConfigToDiskIamge(orignalImage, device, opts.adapter)
  await flash(opts.drive, driveSize, imagePath)

  console.log(`${chalk.bold.green('Done!')}`)
}