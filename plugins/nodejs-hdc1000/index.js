const HDC1000_ERROR = 65535
const HDC1000_TEMP_REG = 0x00
const HDC1000_HUMID_REG = 0x01
const HDC1000_CONFIG_REG = 0x02

class HDC1000 {
  constructor(address = 0x40) {
    this.bus = new I2C({ address })

    this.bus.write([HDC1000_CONFIG_REG, 0x00, 0x00])
    Timer.busywait(500 * 1000)
  }

  read(reg) {
    this.bus.write([reg])

    Timer.busywait(500 * 1000)

    const resp = this.bus.read(2)
    const data = (resp[0] << 8) | resp[1]

    if (data === HDC1000_ERROR) {
      throw new Error('hdc1000 returned an error')
    }

    return data
  }

  read_temperature() {
    return ((this.read(HDC1000_TEMP_REG) / 65536) * 165) - 40
  }

  read_humidity() {
    return (this.read(HDC1000_HUMID_REG) / 65536) * 100
  }
}

module.exports = HDC1000
