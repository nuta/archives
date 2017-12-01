import * as fs from 'fs';
const serial = require(`../../native/${process.arch}/serial.node`)
const { O_RDWR, O_NOCTTY, O_SYNC } = fs.constants

export class SerialAPI {
  path: string;
  watching: boolean;
  fd: number;
  baudrate: number;

  constructor({ path, baudrate }) {
    this.path = path
    this.watching = false
    this.fd = fs.openSync(path, O_RDWR | O_NOCTTY | O_SYNC)
    this.configure(baudrate)
  }

  static list() {
    return fs.readdirSync('/dev')
      .filter(filepath => filepath.match(/^(ttyAMA0|ttyUSB)/))
      .map(filepath => `/dev/${filepath}`)
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

    this.watching = true

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

    this.watching = true

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
