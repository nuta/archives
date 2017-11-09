module.exports = class {
  get OUTPUT() { return 'out' }
  get INPUT() { return 'in' }
  setMode(pin, mode) { }
  write(pin, value) { }
  read(pin) { return 0 }
  onInterrupt(pin, mode, callback) {}
}
