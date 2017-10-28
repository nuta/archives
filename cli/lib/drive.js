const drivelist = require('drivelist')

function getDriveSize(drive) {
  return new Promise((resolve, reject) => {
    drivelist.list((error, drives) => {
      if (error) { reject(error) }

      for (const drive of drives) {
        if (drive.device === drive) {
          resolve(drive.size)
        }
      }

      reject(new Error(`No such a drive: ${drive}`))
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
