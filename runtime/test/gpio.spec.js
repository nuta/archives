const assert = require('power-assert')
const mockfs = require('mock-fs')
const fs = require('fs')
const path = require('path')

process.env.MAKESTACK_DEVICE_TYPE = 'raspberrypi3'
const { builtins } = require('..')

function readFile(path) {
  return fs.readFileSync(path, { encoding: 'utf-8' })
}

describe('GPIO API', function () {
  beforeEach(function () {
    this.pin = 1
    mockfs({
      '/sys/class/gpio/export': '',
      '/sys/class/gpio/gpio1/direction': ''
    })

    this.instance = new builtins.GPIO({
      pin: this.pin,
      mode: builtins.GPIO.OUTPUT
    })
  })

  afterEach(function () {
    mockfs.restore()
  })

  describe('setMode', function () {
    it('changes the direction', function () {
      this.instance.setMode(builtins.GPIO.INPUT)
      assert.strictEqual(readFile('/sys/class/gpio/export'), '1')
      assert.strictEqual(readFile('/sys/class/gpio/gpio1/direction'), 'in')
    })
  })

  describe('write', function () {
    it('changes the value', function () {
      this.instance.write(true)
      assert.strictEqual(readFile('/sys/class/gpio/gpio1/value'), '1')
    })
  })

  describe('read', function () {
    it('reads the value', function () {
      this.instance.write(false)
      assert.strictEqual(this.instance.read(), false)
    })
  })
})
