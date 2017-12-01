import * as fs from 'fs';
import { ioctl } from '../ioctl';

const I2C_SLAVE = 0x0703

export abstract class LinuxI2CAPI {
  abstract path: string;
  address: number;
  fd: number;

  constructor({ address }) {
    this.address = address
    this.fd = fs.openSync(this.path, 'rs+')
  }

  reset() {
    fs.closeSync(this.fd)
  }

  read(length) {
    this.selectSlaveAddress(this.address)
    let buffer = Buffer.alloc(length)
    fs.readSync(this.fd, buffer, 0, length, 0)
    return buffer
  }

  write(data) {
    this.selectSlaveAddress(this.address)
    fs.writeSync(this.fd, Buffer.from(data))
  }

  selectSlaveAddress(address) {
    if (ioctl(this.fd, I2C_SLAVE, address) !== 0) {
      throw new Error('failed to set I2C_SLAVE')
    }
  }
}
