const fs = require('fs')
const serial = require(`../../native/${process.arch}/serial.node`)
const { O_RDWR, O_NOCTTY, O_CLOEXEC, O_SYNC } = fs.constants

class SerialAPI {
  constructor({ path, baudrate }) {
    this.path = path
    this.watching = false
    this.fd = fs.openSync(path, O_RDWR | O_NOCTTY | O_CLOEXEC | O_SYNC)
    this.configure(baudrate)
  }

  configure(baudrate) {
    if (!baudrate) {
      throw new Error("`baudrate' is not speicified")
    }

    this.baudrate = baudrate
    serial.configure(this.fd, baudrate, 0, 0)
  }

  write(data) {
    fs.writeSync(this.fd, data)
  }

  read() {
    return fs.readFileSync(this.fd)
  }

  onData(callback) {
    if (this.watching) {
      throw Error('The serial port is already being watched.')
    }

    this.waching = true

    // FIXME: use libuv
    setInterval(() => {
      const chunk = this.read()
      if (chunk.length > 0) {
        callback(chunk)
      }
    }, 100)
  }

  onNewLine(callback) {
    if (this.watching) {
      throw Error('The serial port is already being watched.')
    }

    this.waching = true

    // FIXME: use libuv
    let buf = ''
    setInterval(() => {
      const chunk = this.read().toString('utf-8')
      if (chunk.length > 0) {
        buf += chunk
        while (buf.includes('\n')) {
          const line = buf.split('\n')[0].replace('\r$', '')
          buf = buf.slice(buf.indexOf('\n') + 1)
          callback(line)
        }
      }
    }, 100)
  }
}

module.exports = SerialAPI
