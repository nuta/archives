class Driver {
  delay(msec) {
    return new Promise((resolve, reject) => {
      setTimeout(() => resolve, msec)
    })
  }
}

module.exports = Driver
