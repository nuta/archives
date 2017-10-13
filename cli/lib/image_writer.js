const fs = require('fs')
const imageWrite = require('etcher-image-write')
const mountutils = require('mountutils')

/*
 *  Be careful! This command will be executed in root!!!
 */

function umount(drive) {
  return new Promise((resolve, reject) => {
    mountutils.unmountDisk(drive, (error) => {
      if (error)
        reject(error)  
      else
        resolve()  
    })
  })
}

module.exports = async () => {
  const drive = process.env.DRIVE
  const driveSize = parseInt(process.env.DRIVE_SIZE)
  const imagePath = process.env.IMAGE_PATH

  if (drive === undefined || driveSize === undefined || imagePath === undefined)
    throw "specify `DRIVE_PATH', `DRIVE_SIZE' and `IMAGE_PATH'"

  console.log('info:', drive, driveSize, imagePath)

  console.log('info:', 'unmounting')
  await umount(drive)

  console.log('info:', 'writing')
  const writer =imageWrite.write({
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
    console.log(`progress: ${JSON.stringify(state)}`)
  })

  writer.on('done', results => {
    console.log(`success: ${JSON.stringify(results)}`)
  })

  writer.on('error', error => {
    console.log(`error: ${error}`)
    process.exit(1)    
  })
}