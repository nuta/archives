const drivelist = require('drivelist')

export function getDriveSize(devfile) {
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


export function getAvailableDrives() {
  return new Promise((resolve, reject) => {
    drivelist.list((error, drives) => {
      if (error) { reject(error) }

      const availableDrives = drives.filter(drive => !drive.system)
      resolve(availableDrives)
    })
  })
}
