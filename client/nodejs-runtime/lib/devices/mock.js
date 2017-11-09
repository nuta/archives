class I2C {
  read(address, length) {
    return Buffer.alloc(length)
  }

  write(address, data) {
  }
}

class GPIO {
  constructor(pin, mode) {
    this.pin = pin
  }

  setMode(mode) { }
  write(value) { }
  read() { return 0 }
  onInterrupt(mode, callback) {}
}

function initialize() {
}

module.exports = { initialize, builtins: { I2C, GPIO } }
