const drivelist = require('drivelist')

function getDriveSize(drive) {
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

module.exports = { getDriveSize, getAvailableDrives }

function getAvailableDrives() {
  return new Promise((resolve, reject) => {
    drivelist.list((error, drives) => {
      if (error)
        reject(error)
      
      const availableDrives = drives.filter(drive => !drive.system)
      resolve(availableDrives)
    })
  })
}