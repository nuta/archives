const fs = require('fs')

module.exports = class {
  get OUTPUT() {
    return 'out'
  }

  get INPUT() {
    return 'in'
  }

  setMode(pin, mode) {
    if (typeof pin !== 'number') {
      throw new Error("`pin' must be a number")
    }

    if (fs.existsSync(`/sys/class/gpio/gpio${pin}`)) { fs.writeFileSync(`/sys/class/gpio/unexport`, `${pin}`) }

    fs.writeFileSync(`/sys/class/gpio/export`, `${pin}`)
    fs.writeFileSync(`/sys/class/gpio/gpio${pin}/direction`,
      (mode === this.INPUT) ? 'in' : 'out')
  }

  write(pin, value) {
    fs.writeFileSync(`/sys/class/gpio/gpio${pin}/value`, value ? '1' : '0')
  }

  read(pin) {
    return fs.readFileSync(`/sys/class/gpio/gpio${pin}/value`, 'utf-8') === '1'
  }

  onInterrupt(pin, mode, callback) {
    callback = (typeof mode === 'function') ? mode : callback
    const edge = mode || 'rising'

    this.setMode(pin, this.INPUT)
    fs.writeFileSync(`/sys/class/gpio/gpio${pin}/edge`, edge)
    fs.watch(`/sys/class/gpio/gpio${pin}/value`, () => {
      callback()
    })
  }
}
