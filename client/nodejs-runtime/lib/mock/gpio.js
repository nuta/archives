module.exports = class {
  constructor({ pin, mode }) {
    this.pin = pin
    this.setMode(mode)
  }

  get OUTPUT() { return 'out' }
  get INPUT() { return 'in' }

  setMode(mode) {
    console.log(`GPIO: setMode(pin=${this.pin}, mode=${mode})`)
  }
  write(value) {
    console.log(`GPIO: write(pin=${this.pin})`)
  }

  read() {
    console.log(`GPIO: read(pin=${this.pin})`)
    return 0
  }

  onInterrupt(mode, callback) {
    console.log(`GPIO: onInterrupt(pin=${this.pin}, mode=${mode})`)
  }

  onChange(callback) {
    console.log(`GPIO: onChange(pin=${this.pin})`)
  }
}
