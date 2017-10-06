const fs = require('fs');

const I2C_SLAVE = 0x0703;

module.exports = class {

  constructor() {
    this.fd = fs.openSync('/dev/i2c-0', 'rw');
  }
  
  get globals() {
    return {
      read: this.read.bind(this),
      write: this.write.bind(this)
    };
  }
  
  reset() {
  }

  read(address, length) {
    selectSlaveAddress(address);
    let buffer = Buffer.alloc(length);
    fs.readSync(this.fd, buffer, 0, length);
    return buffer;
  }

  write(address, data) {
    selectSlaveAddress(address);
    fs.writeSync(this.fd, buffer);
  }

  selectSlaveAddress(address) {
    if (ioctl(this.fd, I2C_SLAVE, address) != 0) {
      throw "failed to open I2C bus";
    }
  }
}
