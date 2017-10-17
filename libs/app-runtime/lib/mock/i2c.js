module.exports = class {
  get globals() {
    return {
      read: this.read.bind(this),
      write: this.write.bind(this)
    }
  }

  reset() {
  }

  read(address, length) {
    return Buffer.alloc(length)
  }

  write(address, data) {
  }
}
