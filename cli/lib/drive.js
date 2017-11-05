const drivelist = require('drivelist')

function getDriveSize(devfile) {
  return new Promise((resolve, reject) => {
    drivelist.list((error, drives) => {
      if (error) { reject(error) }

      for (const drive of drives) {
        if (devfile === drive.device) {
          resolve(drive.size)
        }
      }

      reject(new Error(`No such a drive: ${devfile}`))
    })
  })
}

module.exports = { getDriveSize, getAvailableDrives }

function getAvailableDrives() {
  return new Promise((resolve, reject) => {
    drivelist.list((error, drives) => {
      if (error) { reject(error) }

      const availableDrives = drives.filter(drive => !drive.system)
      resolve(availableDrives)
    })
  })
}
